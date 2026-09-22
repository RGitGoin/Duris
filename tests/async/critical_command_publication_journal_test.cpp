#include "persistence/critical_command_journal.h"
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <vector>
#include <sys/stat.h>
#include <zlib.h>
using bytes = std::vector<uint8_t>;
namespace fs = std::filesystem;
critical_command command(uint8_t tag)
{
	critical_command value{};
	value.operation_id.bytes[0] = tag;
	value.schema_version = 1;
	value.type = critical_command_type::test;
	value.payload_version = 1;
	value.accepted_at_usec = 1700000000000000ULL + tag;
	value.source_site = critical_source_site::command;
	value.deadline_class = critical_deadline_class::interactive;
	value.keys = { { critical_entity_type::player, tag } };
	value.payload = { tag };
	assert(critical_command_normalize(&value));
	return value;
}
struct observation
{
	unsigned calls = 0;
	bool retained;
};
bool observe(critical_command value, bool retained, void *context)
{
	auto &seen = *static_cast<observation *>(context);
	assert(value.payload[0] == 2 && retained == seen.retained);
	++seen.calls;
	return true;
}
bytes read(const fs::path &path)
{
	std::ifstream input(path, std::ios::binary);
	return bytes(std::istreambuf_iterator<char>(input), {});
}
void write(const fs::path &path, const bytes &data)
{
	std::ofstream out(path, std::ios::binary | std::ios::trunc);
	out.write(reinterpret_cast<const char *>(data.data()), data.size());
	assert(out.good());
}
void put(bytes &data, size_t offset, uint64_t value, size_t count)
{
	for (size_t i = 0; i < count; ++i)
		data[offset + i] = uint8_t(value >> (8 * i));
}
int main(int argc, char **argv)
{
	assert(argc == 2);
	const std::string root = argv[1];
	fs::create_directories(root);
	assert(chmod(root.c_str(), 0700) == 0);
	const auto path = fs::path(root) / "critical-command.journal";
	const auto first = command(1), second = command(2);
	assert(critical_command_journal_init(root.c_str()));
	assert(critical_command_journal_append(first) == critical_command_journal_result::ok);
	assert(critical_command_journal_append(second, true) ==
	       critical_command_journal_result::ok);
	assert(critical_command_journal_checkpoint(first.operation_id) ==
	       critical_command_journal_result::ok);
	critical_command_journal_shutdown();
	const auto valid = read(path);
	assert(valid.size() > 41 && valid[4] == 2 && valid[40] == 1);
	assert(critical_command_journal_init(root.c_str()));
	observation seen{ 0, true };
	assert(critical_command_journal_replay_with_publication(observe, &seen) ==
		       critical_command_journal_result::ok &&
	       seen.calls == 1);
	assert(critical_command_journal_append(second, true) ==
	       critical_command_journal_result::ok);
	critical_command_journal_shutdown();
	assert(critical_command_journal_init(root.c_str()));
	seen.calls = 0;
	assert(critical_command_journal_replay_with_publication(observe, &seen) ==
		       critical_command_journal_result::ok &&
	       seen.calls == 1);
	// Conflicting publication metadata cannot silently become a duplicate.
	assert(critical_command_journal_append(second) == critical_command_journal_result::ok);
	critical_command_journal_shutdown();
	assert(!critical_command_journal_init(root.c_str()));
	assert(critical_command_journal_health_copy().last_result ==
	       critical_command_journal_result::corrupt_data);
	critical_command_journal_shutdown();
	for (unsigned variant = 0; variant < 3; ++variant)
	{
		auto corrupt = valid;
		if (variant == 0)
			corrupt.pop_back();
		else if (variant == 1)
			corrupt[40] = 0; // CRC catches changed retention.
		else
		{
			corrupt[40] = 2; // Valid CRC cannot authorize unknown flags.
			put(corrupt, 20, crc32(0, corrupt.data() + 40, corrupt.size() - 40), 4);
		}
		write(path, corrupt);
		assert(!critical_command_journal_init(root.c_str()));
		assert(critical_command_journal_health_copy().last_result ==
		       critical_command_journal_result::corrupt_data);
		critical_command_journal_shutdown();
	}
	// Explicit version-2 false is a valid policy, not an unknown flag.
	auto unretained = valid;
	unretained[40] = 0;
	put(unretained, 20, crc32(0, unretained.data() + 40, unretained.size() - 40), 4);
	write(path, unretained);
	assert(critical_command_journal_init(root.c_str()));
	seen = { 0, false };
	assert(critical_command_journal_replay_with_publication(observe, &seen) ==
		       critical_command_journal_result::ok &&
	       seen.calls == 1);
	critical_command_journal_shutdown();
}
