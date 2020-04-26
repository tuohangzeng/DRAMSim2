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
    //  ./a.out sysini.ini dramini.ini [trace file == Gen ==] [num chan fix 1] [num bank fix 8] [Size log_2 <14 change below, fixed] [cycles] [max FACTOR 1..2..4..] 
    // ./a.out ../system.ini ../ini/DDR3_micron_16M_8B_x8_sg15.ini ==generated== 1 8 ==fixed== 100000 8

    // chan keep at 1
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
    std::string dirName = "results_" + std::to_string(result) + "_C" + argv[4] + "_B" + argv[5] + "_c" + argv[7] + "_maxF" + argv[8];
    std::string cmdStr = "mkdir " + dirName;
    cmd = cmdStr.c_str();
    system(cmd);

    cmdStr = "mkdir " + dirName + "/results";
    cmd = cmdStr.c_str();
    system(cmd);

    cmdStr = "mkdir " + dirName + "/traces";
    cmd = cmdStr.c_str();
    system(cmd);

    cmdStr = "mkdir " + dirName + "/tmp";
    cmd = cmdStr.c_str();
    system(cmd);

    // Adjust if needed

    int transacCount = 50000;
    for (float ratio = 0; ratio <= 1.001; ratio += 0.1) {
        for (int interval = 1; interval <= 30; interval +=1) {
            //std::cout << "traceGen: lambda=" <<  int(exp(0.4*interval)) << ";transactionCount=" << transacCount << ";w/ratio=" << ratio << std::endl;
            cmdStr = "python3 ./traceGen.py " + std::to_string(interval) + " " + std::to_string(transacCount) + " " + std::to_string(ratio) + " ./" + dirName + "/traces";
            cmd = cmdStr.c_str();

            std::cout << "creating trc files" << std::endl;
            std::cout << cmdStr << std::endl << std::endl;
            system(cmd);


            // parse DRAMSim RUN CMD ...
            int totalSize = 4096;
            std::string trcFilePath = " ./" + dirName + "/traces/" + "k6_TraceGen_";
            trcFilePath += std::to_string(interval) + ".0_" + std::to_string(transacCount) + "_" + std::to_string(int(ratio*100)) + ".trc";

            // VALUES ARE FIXED HERE

            for (unsigned int chan = 1; chan <= 1; chan*=2) {      // fixed at 1
                for (unsigned int bank = 8; bank <= 8; bank*=2) {      // fixed at 8
                    for (unsigned int factor = 1; factor <= std::stoi(argv[8]); factor*=2) {    // going from 1 to 8 by *=2
                        // open fstream
                        tmpDram.open("./" + dirName + "/tmp/tmp_dram_run_file.tmp");
                        tmpSys.open("./" + dirName + "/tmp/tmp_sys_run_file.tmp");

                        std::map<std::string, std::string>* sysParameterRT = new std::map<std::string, std::string>;
                        std::map<std::string, std::string>* dramParameterRT = new std::map<std::string, std::string>;

                        //int totalSize = pow(2,size);

                        sysParameterRT->insert(sysParameter->begin(), sysParameter->end());
                        dramParameterRT->insert(dramParameter->begin(), dramParameter->end());

                        (*sysParameterRT)["FACTOR"] = argv[8];
                        //(*dramParameterRT)["NUM_BANKS"] = std::to_string(bank);

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
                        cmdStr = "../DRAMSim -t " + trcFilePath + " -s " + std::string("./") + dirName + "/tmp/tmp_sys_run_file.tmp";
                        cmdStr += " -o FACTOR=" + std::to_string(factor);
                        cmdStr += " -d " + std::string("./") + dirName + "/tmp/tmp_dram_run_file.tmp" + " -c " + std::string(argv[7]) + " -S " + std::to_string(totalSize);  // size should be fixed for experiment
                        cmdStr += " -v " + std::string("./") + dirName + "/results/result" + "_C1" + "_B" + std::to_string(bank) + "_S" + std::to_string(totalSize)+"_F" + std::to_string(factor);
                        cmdStr += "_I" + std::to_string(interval) + ".0_M" + std::to_string(transacCount) + "_R" + std::to_string(int(ratio*100));
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
       }
    }

    
    delete sysParameter;
    delete dramParameter;
}

