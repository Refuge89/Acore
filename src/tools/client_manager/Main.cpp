#include "ClientManager.h"
#include <iostream>

LoginDatabaseWorkerPool LoginDatabase;
UnusedDatabaseWorkerPool UnusedDatabase;
WorldDatabaseWorkerPool WorldDatabase;

std::string FullDataPath;
std::string TinyDataPath;
std::string TinyLocPath;
std::string TinyPatchPath;

std::string FullDataPathEn;
std::string FullDataPathFr;
std::string TinyDataPathEn;
std::string TinyDataPathFr;

fs::path UpdatePath;

int main(int argc, char *argv[])
{
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        // ClientSelector
        ("s-all", "Select All")
        ("s-adt", "Select from adts")
        ("s-db", "Select from the db")
        ("s-sound", "Select sounds")
        ("s-compl", "Selection Completion")
        ("s-noitem", "Don't select items from the db")
        ("s-deldata", "Delete TinyData before selection")
        // ClientCompressor
        ("c-all", "Compress All")
        ("c-alle", "Compress All except Patch")
        ("c-common", "Compress Common")
        ("c-common2", "Compress Common-2")
        ("c-lichking", "Compress Lichking")
        ("c-locale", "Compress Locale")
        ("c-speech", "Compress Speech")
        ("c-patch", "Compress Patch")
        ("c-install", "Move compressed Tiny to game path")
        ("c-release", "Move compressed Tiny to release path + build Wow.exe")
        ("c-udbc", "Update dbc")
        ("c-ulua", "Update lua")
        // ClientPatcher
        ("p-version", "Commit version")
        ("p-update", "Generate update")
        ("p-updatefrom", po::value<uint32>(), "Specify from which build")
        ("p-release", "Copy installers/downloaders to release path")
        ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help"))
    {
        std::cout << desc << "\n";
        return 1;
    }

    std::string configFile = "clientmanager.conf";
    std::string configError;
    if (!sConfigMgr->LoadInitial(configFile, configError))
    {
        printf("Error in config file: %s\n", configError.c_str());
        return 1;
    }

    ClientManager* Manager = new ClientManager(vm);

    printf("\n");
    //printf("\nEndOfExe\n  ");
    //system("pause");
    return 0;
}
