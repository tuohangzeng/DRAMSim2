#include <iostream>
#include <stdlib.h>
#include <vector>
#include <string>
#include <fstream>
#include <utility>
#include <cmath>
#include <map>
#include <ctime>

void iniReader(std::map<std::string, std::string>* parameter, std::ifstream &iniFile) {
    unsigned int lineNumber = 0;
    std::string line = "";
    signed int commentIndex = -1;
    signed int equalsIndex = -1;
    std::string key, valueString;

    while (!iniFile.eof())
		{
			lineNumber++;
			std::getline(iniFile, line);
			// skip zero-length lines
			if (line.size() == 0)
			{
				continue;
			}
			//search for a ;
			if ((commentIndex = line.find_first_of(";")) != std::string::npos)
			{
				//if ; is the first char, ignore the whole line
				if (commentIndex == 0)
				{
					continue;
				}

				line = line.substr(0,commentIndex);
			}

			size_t whiteSpaceEndIndex;
			if ((whiteSpaceEndIndex = line.find_last_not_of(" \t")) != std::string::npos)
			{
				line = line.substr(0,whiteSpaceEndIndex+1);
			}

			// find = sign
			if ((equalsIndex = line.find_first_of("=")) == std::string::npos)
			{
				std::cerr << "Malformed Line "<<lineNumber<<" (missing equals)";
				abort();
			}
			size_t strlen = line.size();
			key = line.substr(0, equalsIndex);
			valueString = line.substr(equalsIndex+1,strlen-equalsIndex);

            // store into map
			(*parameter)[key] = valueString;
		}
}

int main(int argc, char* argv[]) {
    //  ./a.out sysini.ini dramini.ini trace.trc [num chan] [num bank] [Size log_2 <14] [cycles]
    // ./a.out ../system.ini ../ini/DDR3_micron_16M_8B_x8_sg15.ini ../traces/k6_aoe_02_short.trc 4 4 13 10000

    const char *cmd = 0;
    std::ifstream sysIniFile(argv[1]);
    if (sysIniFile.bad()) {
        std::cerr << "Error opening the sysIni File";
    }
    std::ifstream dramIniFile(argv[2]);
    if (dramIniFile.bad()) {
        std::cerr << "Error opening the dramIni File";
    }
    std::ofstream tmpDram;
    std::ofstream tmpSys;
    
    std::map<std::string, std::string>* sysParameter = new std::map<std::string, std::string>;
    std::map<std::string, std::string>* dramParameter = new std::map<std::string, std::string>;

    // pass the stream and map pointers to the parser
    iniReader(sysParameter, sysIniFile);
    iniReader(dramParameter, dramIniFile);

    // mkdir
    std::time_t result = std::time(nullptr);
    std::string dirName = "results_" + std::to_string(result) + "_C" + argv[4] + "_B" + argv[5] + "_S2^" + argv[6] + "_C" + argv[7];
    std::string cmdStr = "mkdir " + dirName;
    cmd = cmdStr.c_str();
    system(cmd);

    for (unsigned int chan = 1; chan <= std::stoi(argv[4]); chan*=2) {
        for (unsigned int bank = 1; bank <= std::stoi(argv[5]); bank*=2) {
            for (unsigned int size = 1; size <= std::stoi(argv[6]); size++) {
                // open fstream
                tmpDram.open("tmp_dram_run_file.tmp");
                tmpSys.open("tmp_sys_run_file.tmp");

                std::map<std::string, std::string>* sysParameterRT = new std::map<std::string, std::string>;
                std::map<std::string, std::string>* dramParameterRT = new std::map<std::string, std::string>;

                int totalSize = pow(2,size);

                sysParameterRT->insert(sysParameter->begin(), sysParameter->end());
                dramParameterRT->insert(dramParameter->begin(), dramParameter->end());

                (*sysParameterRT)["NUM_CHANS"] = std::to_string(chan);
                (*dramParameterRT)["NUM_BANKS"] = std::to_string(bank);

                // regenerate file to tmp
                for (std::map<std::string,std::string>::iterator it=sysParameterRT->begin(); it!=sysParameterRT->end(); ++it) {
                    tmpSys << it->first << "=" << it->second << '\n';
                }
                for (std::map<std::string,std::string>::iterator it=dramParameterRT->begin(); it!=dramParameterRT->end(); ++it) {
                    tmpDram << it->first << "=" << it->second << '\n';
                }

                // fstream close
                tmpDram.close();
                tmpSys.close();
                
                // parse run comands (pay attention to totalSize and vis file output)
                cmdStr = "../DRAMSim -t " + std::string(argv[3]) + " -s " + "./tmp_sys_run_file.tmp";
                cmdStr += " -d " + std::string("./tmp_dram_run_file.tmp") + " -c " + std::string(argv[7]) + " -S " + std::to_string(totalSize);
                cmdStr += " -v " + std::string("./") + dirName + "/result_" + std::to_string(result) + "_C" + std::to_string(chan) + "_B" + std::to_string(bank) + "_S" + std::to_string(totalSize);
                cmdStr += " > /dev/null 2>&1" ;     // throw the output away for silent execution

                cmd = cmdStr.c_str();
                std::cout << "Executing cmd: " << cmd << std::endl;
                // run the command
                system(cmd);

                // delete maps
                delete sysParameterRT;
                delete dramParameterRT;
            }
        }
    }
    delete sysParameter;
    delete dramParameter;
}

