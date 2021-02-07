#pragma once

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>



class File {
private:
	// members
	std::fstream m_file_origin;
	std::vector<std::string> m_file_content;

	std::string m_file_name;
	
	unsigned m_line_count;
	double m_file_size;


	// checks if current row should be deleted
	bool check_if_to_delete(std::vector<unsigned>& to_delete, unsigned line) {

		for (auto& i : to_delete) {
			if (i == line) { return true; }
		}
		return false;
	}


	// calculates and prints out the size of the file
	void calculate_file_size() {

		std::string suffix = " B";
		
		if (m_file_size > std::pow(10.0, 9.0)) {
			suffix = " GB";
			m_file_size /= std::pow(10.0, 9.0);
		}
		else if (m_file_size > std::pow(10.0, 6.0)) {
			suffix = " MB";
			m_file_size /= std::pow(10.0, 6.0);
		}
		else if (m_file_size > std::pow(10.0, 3.0)) {
			suffix = " kB";
			m_file_size /= std::pow(10.0, 3.0);
		}
		// round to two decimal places
		std::cout << std::fixed;
		std::cout << std::setprecision(2);
		std::cout << "File size: " << m_file_size << suffix << "\n";
	}


public:
	// constructor
	File(const char* file_name) {

		// open existing file or ...
		m_file_name = file_name;
		m_file_origin.open(m_file_name);

		// ... creates new file if needed
		if (!m_file_origin.is_open()) {
			m_file_origin.open(m_file_name, std::ios::trunc | std::ios::in | std::ios::out);
		}
	}

	// destructor
	~File() = default;

	
	// getters and setters
	void set_line_count(unsigned line_count) { m_line_count = line_count; }

	unsigned get_line_count() {	return m_line_count; }

	void set_file_content(std::vector<std::string>& file_content) {	m_file_content = file_content; }

	std::vector<std::string> get_file_content() {	return m_file_content; }



	// prints selected lines from file
	void print_lines(std::vector<unsigned>& to_print) {

		// print whole file
		if (to_print.empty()) {
			for (auto& i : m_file_content) {
				std::cout << i << "\n";
			}
		}
		// print lines in range
		else {
			// checks range validity
			if (to_print.back() > m_line_count || std::count(to_print.begin(), to_print.end(), 0u)) {
				std::cout << "Error: Invalid range\n";
				return;
			}
			for (auto i : to_print) {
				std::cout << m_file_content[i - 1] << "\n";
			}
		}
	}


	// deletes selected lines from file
	void delete_lines(std::vector<unsigned>& to_delete) {

		// deletes whole file
		if (to_delete.empty()) {
			std::vector<std::string> empty;
			set_file_content(empty);
		}
		// delete lines in range
		else {
			// checks range validity
			if (to_delete.back() > m_line_count || std::count(to_delete.begin(), to_delete.end(), 0u)) {
				std::cout << "Error: Invalid range\n";
				return;
			}
			std::vector<std::string> temp;
			for (unsigned i = 1; i <= m_file_content.size(); ++i) {
				if (!check_if_to_delete(to_delete, i)) {
					temp.push_back(m_file_content[i - 1]);
				}
			}
			// saves made changes
			set_file_content(temp);
			set_line_count(static_cast<unsigned>(temp.size()));
		}
	}


	// changes selected lines in file with user input
	void change_lines(std::vector<unsigned>& to_change) {

		// replaces whole file
		if (to_change.empty()) {

			std::vector<std::string> temp;
			std::string user_input;
			m_line_count = 0;

			while (true) {
				std::getline(std::cin, user_input);
				if (user_input == ".") { break; }

				temp.push_back(user_input);
				++m_line_count;
			}
			set_file_content(temp);
		}
		// delete lines in range and append after line before the first one in range (if vector does not contain zero)
		else if (!std::count(to_change.begin(), to_change.end(), 0u)  && to_change.back() <= m_line_count){
			auto index = to_change[0] - 1;
			delete_lines(to_change);
			append(index);
		}
		else {
			std::cout << "Error: Invalid range\n";
			return;
		}
	}


	// saves file to disk
	void save_file() {

		// opens original file and deletes it's content
		m_file_origin.open(m_file_name, std::ios::trunc | std::ios::out);

		if (!m_file_origin.is_open()) {
			std::cout << "Error: File coudn't be saved\n";
			return;
		}
		// writes to file
		for (size_t i = 0; i < m_file_content.size(); ++i) {

			// don't add new line when we're at the end
			if (i == (m_file_content.size() - 1)) {
				m_file_origin << m_file_content[i];
			}
			else {
				m_file_origin << m_file_content[i] << "\n";
			}
		}

		// calculates file size
		m_file_origin.seekg(0, std::ios::end);
		m_file_size = static_cast<double>(m_file_origin.tellg());
		calculate_file_size();

		m_file_origin.close();
	}


	// compares made changes, returns true if original and modified file are the same
	bool compare_changes() {

		m_file_origin.open(m_file_name, std::ios::in);
		
		// original file is loaded into temp vector
		std::vector<std::string> temp;
		std::string row;
		while (std::getline(m_file_origin, row)) {
			temp.push_back(row);
		}
		m_file_origin.close();

		// compares if they are equal (same size and elements)
		if (m_file_content == temp) {
			return true;
		}
		else {
			return false;
		}
	}


	// appends to file
	void append(std::optional<unsigned> line_num) {
		
		if (line_num.has_value()) {
			auto index = static_cast<int>(line_num.value());

			// creates empty lines if needed
			while (line_num.value() > m_line_count) {
				m_file_content.push_back("");
				++m_line_count;
			}

			std::string user_input;

			while (true) {
				std::getline(std::cin, user_input);
				if (user_input == ".") { break; }

				m_file_content.insert(m_file_content.begin() + index, user_input);
				++m_line_count;
				++index;
			}
		}
		else {
			std::cout << "Error: Invalid range\n";
			return;
		}
	}


	// loads file content into vector, sets up number of lines
	void load_file() {

		std::string row;
		while (std::getline(m_file_origin, row)) {
			m_file_content.push_back(row);
		}
		if (!m_file_origin.eof()) {
			std::cout << "Error: File was not loaded completely\n";
			exit(1);
		}
		m_file_origin.close();
		m_line_count = static_cast<unsigned>(m_file_content.size());
	}
};
