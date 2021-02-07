#pragma once


#include <optional>
#include <exception>

#include "File.h"



class Command {
private:
	// members
	std::string m_command;
	char m_command_final;
	std::vector<unsigned> m_range;
	
	size_t m_range_min;
	size_t m_range_max;
	
	bool m_all_lines;
	bool m_is_range;
	std::pair<bool, bool> m_limits;


	// creates vector of selected lines
	void select_lines_in_range(std::vector<unsigned> range_limits, unsigned line_count) {

		// one line selected or special conditions intervals
		if (range_limits.size() == 1) {

			// one line selected
			if (!m_limits.first && !m_limits.second) {
				m_range.push_back(range_limits[0]);
				return;
			}
			if (std::count(range_limits.begin(), range_limits.end(), 0u)) {
				m_range.push_back(range_limits[0]);
			}
			// interval from start of the document to some line
			else if (m_limits.first) {
				for (unsigned i = 1; i <= range_limits[0]; ++i) {
					m_range.push_back(i);
				}
			}
			// interval from some line to the end of the document
			else if (m_limits.second) {
				if (range_limits[0] < line_count) {
					for (auto i = range_limits[0]; i <= line_count; ++i) {
						m_range.push_back(i);
					}
				}
				else {
					m_range.push_back(range_limits[0]);
				}
			}
		}
		// interval selected (not special conditions)
		else if (range_limits.size() == 2) {
			m_range_min = range_limits[0];
			m_range_max = range_limits[1];

			if (m_range_min > m_range_max) {
				throw std::out_of_range("Error: Invalid range\n");
			}
			for (auto i = m_range_min; i <= m_range_max; ++i) {
				m_range.push_back(static_cast<int>(i));
			}
		}
		else {
			throw std::out_of_range("Error: Invalid range\n");
		}
	}


	// sets up current range
	void range_setup(std::string& range, unsigned line_count) {
		

		// checks special conditions (all lines, from the first line, to the last line)
		if (range == ",") {
			m_all_lines = true;
		}
		else if (range.front() == ',') {
			m_limits.first = true;
			m_limits.second = false;
		}
		else if (range.back() == ',') {
			m_limits.first = false;
			m_limits.second = true;
		}

		// checks if range contains only valid characters
		if (range.find_first_not_of(",0123456789") != std::string::npos || std::count(range.begin(), range.end(), ',') > 1) {
			throw std::out_of_range("Error: Invalid range\n");
		}
		std::replace(range.begin(), range.end(), ',', ' ');

		std::stringstream ss(range);
		std::vector<unsigned> range_limits;
		unsigned temp;

		while (ss >> temp) {
			range_limits.push_back(temp);
		}
		if (!range_limits.empty()) {
			select_lines_in_range(range_limits, line_count);
		}
	}


	// sets up correct line when appending to file
	std::optional<unsigned> select_line(unsigned line_count) {

		// if invalid range was selected
		if (m_all_lines || m_limits.first || m_limits.second || m_range.size() > 1) {
			return std::nullopt;
		}
		// if line was selected
		else if (m_range.size() == 1) {
			return m_range[0];
		}
		// append after last line
		else {
			return line_count;
		}
	}


	// checks if command is valid, recommend others if possible
	void check_command_validity() {

		// vectors containing all possible commands and their aliases
		std::vector<std::string> cmd_print = { "print", "vypis", "ukaz", "show", "vytlac" };
		std::vector<std::string> cmd_delete = { "delete", "zmaz", "odstran", "wipe" };
		std::vector<std::string> cmd_change = { "change", "zmenit", "modify", "vymen" };
		std::vector<std::string> cmd_append = { "append", "paste", "dopis" };
		std::vector<std::string> cmd_write = { "write", "zapis", "save", "uloz" };
		std::vector<std::string> cmd_quit = { "quit", "odist", "ukoncit", "exit" };

		std::vector<std::pair<std::vector<std::string>, char>> commands = { 
			std::make_pair(cmd_print, 'p'), 
			std::make_pair(cmd_delete, 'd'),
			std::make_pair(cmd_change, 'c'),
			std::make_pair(cmd_append, 'a'),
			std::make_pair(cmd_write, 'w'),
			std::make_pair(cmd_quit, 'q')
		};
		std::vector<std::pair<std::string, char>> possibilities;

		// remove '!' if command contains it
		std::string command = m_command;
		if (m_command.size() > 0 && m_command[m_command.size() - 1] == '!') {
			command.pop_back();
		}

		// find all possible options
		for (auto& cmd : commands) {
			for (size_t i = 0; i < cmd.first.size(); ++i) {
				if (cmd.first[i].compare(0, command.size(), command) == 0) {
					possibilities.push_back(std::make_pair(cmd.first[i], cmd.second));
				}
			}
		}
		// if there are multiple possible options
		if (possibilities.size() > 1 && !command.empty()) {
			m_command_final = 'x';
			std::cout << "Ambiguous command, possible options are: ";
			for (auto& i : possibilities) {
				std::cout << i.first << " ";
			}
			std::cout << "\n";
		}
		// if there is only one option
		else if (possibilities.size() == 1) {
			m_command_final = possibilities[0].second;
			
			// setup force quit command
			if (m_command_final == 'q' && m_command[m_command.size() - 1] == '!') {
				m_command_final = 'f';
			}
			else if (m_command[m_command.size() - 1] == '!') {
				m_command_final = 'x';
				std::cout << "Invalid command, you probably meant this: " << possibilities[0].first << "\n";
			}
		}
		// if there is no option
		else {
			m_command_final = 'x';
			std::cout << "Error: Unknown command\n";
		}
	}


public:
	// constructor
	Command(std::string& input, unsigned line_count) {

		try {
			std::string buffer;
			std::stringstream ss(input);
			std::vector<std::string> tokens;

			// splits input into different words (tokens)
			while (ss >> buffer) {
				tokens.push_back(buffer);
			}

			// check if size is valid (command and range (optional))
			if (tokens.empty() || tokens.size() > 2) {
				throw std::invalid_argument("Error: Invalid command\n");
			}

			// sets up current command
			m_command = tokens[0];

			// sets up range metadata as false
			m_all_lines = false;
			m_limits.first = false;
			m_limits.second = false;
			m_is_range = false;


			// sets up current range (optional)
			if (tokens.size() == 2) {
				m_is_range = true;
				range_setup(tokens[1], line_count);
			}
		}
		catch (const std::exception& e) {
			throw e;
		}
	}

	// destructor
	~Command() = default;

	// runs command, returns nullopt when user wants to quit
	std::optional<int> run_command(File& file) {

		check_command_validity();

		switch (m_command_final) {
		// print
		case ('p'):
			file.print_lines(m_range);
			break;

		// delete
		case ('d'):
			file.delete_lines(m_range);
			break;

		// append
		case ('a'):
			file.append(select_line(file.get_line_count()));
			break;

		// change
		case ('c'):
			file.change_lines(m_range);
			break;

		// write
		case ('w'):
			if (!m_is_range) {
				file.save_file();
			}
			else {
				std::cout << "Error: Invalid command, remove range\n";
			}
			break;

		// quit
		case ('q'):
			if (!m_is_range) {
				if (file.compare_changes()) {
					return std::nullopt;
				}
				else {
					std::cout << "Error:  Can't quit, unsaved progress would be lost."
						"\n\tEither save made changes (* write) or force quit (* quit!)\n";
				}
			}
			else {
				std::cout << "Error: Invalid command, remove range\n";
			}
			break;

		// force quit
		case ('f'):
			if (!m_is_range) {
				std::cout << "Warning: Force quit, unsaved progress will be lost\n";
				return std::nullopt;
			}
			else {
				std::cout << "Error: Invalid command, remove range\n";
			}
			break;
		}
		return 0;
	}
};