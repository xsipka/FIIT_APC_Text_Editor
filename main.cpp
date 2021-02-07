
#include "Command.h"



/* Implementovane bonusy:
*	-  Human readable size
*	-  Shell
*	-  Multi language & fuzzy support
*/


// main
int main(int argc, char* argv[]) {


	if (argc == 2) {

		File file(argv[1]);
		file.load_file();

		std::string user_input;

		// main loop
		while (true) {

			std::cout << "\n* ";
			std::getline(std::cin, user_input);

			// read and process commands until user quits
			try {
				
				// remove whitespaces before first word if needed
				auto pos = user_input.find_first_not_of(' ');
				user_input = user_input.substr(pos != std::string::npos ? pos : 0);

				// if shell command
				if (user_input.size() > 2 && user_input[0] == '!') {
					user_input.erase(user_input.begin());
					system(user_input.c_str());
				}
				// if program command
				else {
					Command current(user_input, file.get_line_count());
					if (current.run_command(file) == std::nullopt) { break; }
				}
			}
			catch (const std::exception& e) {
				std::cout << e.what();
			}
		}
	}
	else {
		std::cout << "Error: Invalid number of parameters\n";
		return 1;
	}	

	return 0;
}
