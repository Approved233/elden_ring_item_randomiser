#include "item_randomiser_main.h"
#include <iostream>

ERItemRandomiser* main_mod = nullptr;

void ERItemRandomiserLoader::CreateModFunctions() {

	using namespace std::chrono_literals;

	// Wait for Elden Ring
	std::this_thread::sleep_for(5s);

	main_mod = new ERItemRandomiser();

	main_mod->RunSaveListener();

	delete main_mod;
	return;
};

void ERItemRandomiser::RunSaveListener() {

	using namespace std::chrono_literals;

	if (ITEM_DEBUG) {
		AllocConsole();
		freopen("CONOUT$", "w", stdout);
	};

	if (!GetUserPreferences()) {
		//
	};

#if ITEM_DEBUG
	try {
#endif
		hook_class = ERRandomiserBase(user_preferences & option_autoequip, user_preferences & option_randomisekeys, user_preferences & option_randomiseestusupgrades,
			user_preferences & option_randomisemtrlupgrades, user_preferences & option_skipsaveextension, randomiser_seed);

		if (!hook_class.CreateMemoryEdits()) {
			//
			return;
		};
#if ITEM_DEBUG
	}
	catch (std::exception& ex) {
		std::cout << "Hook Failed: " << ex.what() << "\n";
	}
#endif

	while (is_mod_active) {

		std::this_thread::sleep_for(5s);
		/*
		std::unique_lock<std::mutex> unique_lock(save_request_mutex);
		save_request_wait.wait(unique_lock, [&] { return save_manager_wake; });
		if (signal_list_save) {
			// Save... (no longer needed)
		};
		*/
	};

	return;
};

bool ERItemRandomiser::GetUserPreferences() {

	// INIReader
	INIReader option_reader = INIReader("ItemRandomiser//randomiserpreferences.ini");
	int error = option_reader.ParseError();
	if (error) {
		//
		return false;
	};

	std::string header_segment = "MOD";
	user_preferences = option_reader.GetBoolean(header_segment, "autoequip", true) ? static_cast<UserPreferences>(user_preferences | option_autoequip) : user_preferences;
	user_preferences = option_reader.GetBoolean(header_segment, "randomisekeys", false) ? static_cast<UserPreferences>(user_preferences | option_randomisekeys) : user_preferences;
	user_preferences = option_reader.GetBoolean(header_segment, "randomiseflaskmaterials", true) ? static_cast<UserPreferences>(user_preferences | option_randomiseestusupgrades) : user_preferences;
	user_preferences = option_reader.GetBoolean(header_segment, "randomiseupgradematerials", true) ? static_cast<UserPreferences>(user_preferences | option_randomisemtrlupgrades) : user_preferences;
	user_preferences = option_reader.GetBoolean(header_segment, "skipsaveextension", false) ? static_cast<UserPreferences>(user_preferences | option_skipsaveextension) : user_preferences;

	// Param randomisation preferences
	header_segment = "RANDOMISE";
	for (size_t q = 0; q < main_mod->param_container_names.size(); q++) {
		std::string segment(main_mod->param_container_names.at(q).begin(), main_mod->param_container_names.at(q).end());
		RandomiseType randomise_type = static_cast<RandomiseType>(option_reader.GetInteger(header_segment, segment, 0));
		SetParamRandomisationPreference(main_mod->param_container_names.at(q), randomise_type);
	};

	// Seed
	OFSTRUCT file_struct = {};
	HFILE seed_file = OpenFile("ItemRandomiser//randomiser_seed.txt", &file_struct, OF_READWRITE);
	if (seed_file == HFILE_ERROR) {
		char to_write_seed[24] = {};
		LARGE_INTEGER timestamp_counter = {};
		QueryPerformanceCounter(&timestamp_counter);
		int bytes_size = sprintf_s(to_write_seed, "%016llX", timestamp_counter.QuadPart);
		randomiser_seed = timestamp_counter.QuadPart;

		HANDLE new_file_creation = CreateFileA("ItemRandomiser//randomiser_seed.txt", GENERIC_READ | GENERIC_WRITE, 0,
			nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, 0);

		if (new_file_creation == INVALID_HANDLE_VALUE) {

			return false;
		};

		if (!WriteFile(new_file_creation, to_write_seed, bytes_size, nullptr, nullptr)) {

			return false;
		};

		CloseHandle(new_file_creation);
		return true;
	}
	else {
		char file_contents[MAX_PATH] = {};
		DWORD size = 0;
		if (!ReadFile((HANDLE)seed_file, file_contents, sizeof(file_contents), &size, nullptr) || size < 16) {

			return false;
		};
		uint64_t seed = std::strtoull(file_contents, nullptr, 16);
		randomiser_seed = seed;
		CloseHandle((HANDLE)seed_file);
	};

	return true;
};

void ERItemRandomiser::RequestItemListSave(bool request_save) {
	signal_list_save = request_save;
	save_manager_wake = true;
	save_request_wait.notify_one();
	return;
};

void ERItemRandomiser::SetParamRandomisationPreference(std::wstring param_name, RandomiseType option) {
	param_container_random_preferences.emplace(param_name, option);
	return;
};

RandomiseType ERItemRandomiser::GetParamRandomisationPreference(std::wstring param_name) {
	auto random_preference = param_container_random_preferences.find(param_name);
	if (random_preference != param_container_random_preferences.end()) {
		return random_preference->second;
	}
	return randomisetype_none;
};

uint32_t ERItemRandomiser::GetRandomUint(uint32_t min, uint32_t max) {
	std::random_device random_number_device;
	std::mt19937 random_number_generator(random_number_device());
	std::uniform_int_distribution<std::mt19937::result_type> random_number_distributer(min, max);
	return random_number_distributer(random_number_generator);
};

uint32_t ERItemRandomiser::GetSeededRandomUint(uint32_t min, uint32_t max, uint32_t seed) {
	std::mt19937 random_number_generator(seed);
	std::uniform_int_distribution<std::mt19937::result_type> random_number_distributer(min, max);
	return random_number_distributer(random_number_generator);
};

UserPreferences ERItemRandomiser::GetToggleOptions() {
	return user_preferences;
};