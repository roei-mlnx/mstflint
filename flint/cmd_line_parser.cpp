/*
 *
 * cmd_line_parser.cpp - FLash INTerface
 *
 * Copyright (c) 2013 Mellanox Technologies Ltd.  All rights reserved.
 *
 * This software is available to you under a choice of one of two
 * licenses.  You may choose to be licensed under the terms of the GNU
 * General Public License (GPL) Version 2, available from the file
 * COPYING in the main directory of this source tree, or the
 * OpenIB.org BSD license below:
 *
 *     Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 *  Version: $Id$
 *
 */

#include <iostream>
#include <stdio.h>
#include <errno.h>
// Flint includes
#include "flint.h"
#include <common/tools_version.h>
#include "mlxfwops/lib/flint_io.h"

#ifndef FLINT_NAME
    #ifdef __GNUC__
        #define FLINT_NAME "flint"
    #else
        #define FLINT_NAME "flint"
    #endif
#endif

using namespace std;

/******************************
* Data structures containing
* Meta data about subcommands
* and flags
******************************/
class SubCmdMetaData {
private:
    class SubCmd {
public:
        SubCmd(string s, string l, sub_cmd_t n) :  shortf(s), longf(l), cmdNum(n) {}
        ~SubCmd(){}
        string shortf;
        string longf;
        sub_cmd_t cmdNum;
    };
    vector<class SubCmd*> _sCmds;
public:
    SubCmdMetaData();
    ~SubCmdMetaData();
    sub_cmd_t getCmdNum(string flag);
};

SubCmdMetaData::SubCmdMetaData()
{
    _sCmds.push_back(new SubCmd("b", "burn", SC_Burn));
    _sCmds.push_back(new SubCmd("q", "query", SC_Query));
    _sCmds.push_back(new SubCmd("v", "verify", SC_Verify));
    _sCmds.push_back(new SubCmd("", "swreset", SC_Swreset));
    _sCmds.push_back(new SubCmd("", "reset_cfg", SC_ResetCfg));
    _sCmds.push_back(new SubCmd("", "brom", SC_Brom));
    _sCmds.push_back(new SubCmd("", "drom", SC_Drom));
    _sCmds.push_back(new SubCmd("", "rrom", SC_Rrom));
    _sCmds.push_back(new SubCmd("", "qrom", SC_Qrom));
    _sCmds.push_back(new SubCmd("", "bb", SC_Bb));
    _sCmds.push_back(new SubCmd("", "sg", SC_Sg));
#ifndef EXTERNAL
    _sCmds.push_back(new SubCmd("", "smg", SC_Smg));
    _sCmds.push_back(new SubCmd("", "set_vpd", SC_Set_Vpd));
#endif
    _sCmds.push_back(new SubCmd("", "sv", SC_Sv));
    _sCmds.push_back(new SubCmd("", "ri", SC_Ri));
    _sCmds.push_back(new SubCmd("", "dc", SC_Dc));
    _sCmds.push_back(new SubCmd("", "dh", SC_Dh));
    _sCmds.push_back(new SubCmd("", "set_key", SC_Set_Key));
    _sCmds.push_back(new SubCmd("", "hw_access", SC_Hw_Access));
    _sCmds.push_back(new SubCmd("", "hw", SC_Hw));
    _sCmds.push_back(new SubCmd("e", "erase", SC_Erase));
    _sCmds.push_back(new SubCmd("", "rw", SC_Rw));
    _sCmds.push_back(new SubCmd("", "ww", SC_Ww));
    _sCmds.push_back(new SubCmd("", "wwne", SC_Wwne));
    _sCmds.push_back(new SubCmd("", "wbne", SC_Wbne));
    _sCmds.push_back(new SubCmd("", "wb", SC_Wb));
    _sCmds.push_back(new SubCmd("", "rb", SC_Rb));
    _sCmds.push_back(new SubCmd("", "clear_semaphore", SC_Clear_Sem));
    _sCmds.push_back(new SubCmd("", "fi", SC_Fix_Img));
    _sCmds.push_back(new SubCmd("cs", "checksum", SC_Check_Sum));
    _sCmds.push_back(new SubCmd("ts", "timestamp", SC_Time_Stamp));
    _sCmds.push_back(new SubCmd("ci", "cache_image", SC_Cache_Image));
    _sCmds.push_back(new SubCmd("", "sign", SC_Sign));
    _sCmds.push_back(new SubCmd("", "sign_with_hmac", SC_Add_Hmac));
    _sCmds.push_back(new SubCmd("", "extract_fw_data", SC_Extract_4MB_Image));
    _sCmds.push_back(new SubCmd("", "set_public_keys", SC_Set_Public_Keys));
    _sCmds.push_back(new SubCmd("", "set_forbidden_versions", SC_Set_Forbidden_Versions));
    _sCmds.push_back(new SubCmd("ir", "image_reactivation", SC_Image_Reactivation));
    _sCmds.push_back(new SubCmd("bc", "binary_compare", SC_Binary_Compare));
}

SubCmdMetaData::~SubCmdMetaData()
{
    for (vector<class SubCmd*>::iterator it = _sCmds.begin(); it != _sCmds.end(); ++it) {
        delete *it;
    }
}

sub_cmd_t SubCmdMetaData::getCmdNum(string flag)
{
    for (vector<class SubCmd*>::iterator it = _sCmds.begin(); it != _sCmds.end(); ++it) {
        if (((*it)->shortf == flag) || ((*it)->longf == flag)) {
            return (*it)->cmdNum;
        }
    }
    return SC_No_Cmd;
}


class FlagMetaData {
private:
    class Flag {
public:
        Flag(string s, string l, int n) :  shortf(s), longf(l), argNum(n) {}
        ~Flag(){}
        string shortf;
        string longf;
        int argNum;
    };
    vector<class Flag*> _flags;
public:
    FlagMetaData();
    ~FlagMetaData();
    int getNumOfArgs(string flag);
};

FlagMetaData::FlagMetaData()
{
    _flags.push_back(new Flag("d", "device", 1));
    _flags.push_back(new Flag("", "guid", 1));
    _flags.push_back(new Flag("", "guids", GUIDS));
    _flags.push_back(new Flag("", "mac", 1));
    _flags.push_back(new Flag("", "macs", MACS));
    _flags.push_back(new Flag("", "uid", 1));
    _flags.push_back(new Flag("", "blank_guids", 0));
    _flags.push_back(new Flag("", "clear_semaphore", 0));
    _flags.push_back(new Flag("h", "help", 0));
    _flags.push_back(new Flag("", "hh", 0));
    _flags.push_back(new Flag("i", "image", 1));
    _flags.push_back(new Flag("", "qq", 0));
    _flags.push_back(new Flag("", "low_cpu", 0));
    _flags.push_back(new Flag("", "next_boot_fw_ver", 0));
    _flags.push_back(new Flag("", "flashed_version", 0));
    _flags.push_back(new Flag("", "nofs", 0));
    _flags.push_back(new Flag("", "allow_psid_change", 0));
    _flags.push_back(new Flag("", "allow_rom_change", 0));
    _flags.push_back(new Flag("", "override_cache_replacement", 0));
    _flags.push_back(new Flag("", "ocr", 0));
    _flags.push_back(new Flag("", "no_flash_verify", 0));
    _flags.push_back(new Flag("s", "silent", 0));
    _flags.push_back(new Flag("y", "yes", 0));
    _flags.push_back(new Flag("", "no", 0));
    _flags.push_back(new Flag("", "vsd", 1));
    _flags.push_back(new Flag("", "use_image_ps", 0));
    _flags.push_back(new Flag("", "use_image_guids", 0));
    _flags.push_back(new Flag("", "use_image_rom", 0));
    _flags.push_back(new Flag("", "ignore_dev_data", 0));
    _flags.push_back(new Flag("", "dual_image", 0));
    _flags.push_back(new Flag("", "striped_image", 0));
    _flags.push_back(new Flag("", "banks", 1));
    _flags.push_back(new Flag("", "log", 1));
    _flags.push_back(new Flag("", "flash_params", 1)); //its actually 3 but separated by comma so we refer to them as one
    _flags.push_back(new Flag("v", "version", 0));
    _flags.push_back(new Flag("", "no_devid_check", 0));
    _flags.push_back(new Flag("", "use_fw", 0));
    _flags.push_back(new Flag("", "use_dev_img_info", 0));
    _flags.push_back(new Flag("", "skip_ci_req", 0));
    _flags.push_back(new Flag("", "use_dev_rom", 0));
    _flags.push_back(new Flag("", "private_key", 1));
    _flags.push_back(new Flag("", "key_uuid", 1));
    _flags.push_back(new Flag("", "private_key2", 1));
    _flags.push_back(new Flag("", "hmac_key", 1));
    _flags.push_back(new Flag("", "key_uuid2", 1));
    _flags.push_back(new Flag("", "no_fw_ctrl", 0));
    _flags.push_back(new Flag("image_reactivation", "ir", 0));
}

FlagMetaData::~FlagMetaData()
{
    for (vector<class Flag*>::iterator it = _flags.begin(); it != _flags.end(); ++it) {
        delete *it;
    }
}

int FlagMetaData::getNumOfArgs(string flag)
{
    for (vector<class Flag*>::iterator it = _flags.begin(); it != _flags.end(); ++it) {
        if (((*it)->shortf == flag) || ((*it)->longf == flag)) {
            return (*it)->argNum;
        }
    }
    return -1;
}

/********************
 * Helper Functions:
 *******************/

// NOTE : number of parameters extracted in the container should be checked.
static void splitByDelimiters(std::vector<string>& container, string str, const char *delimiters )
{
    if (str.size() == 0) {
        return;
    }
    char *cStr = strcpy(new char[str.size() + 1], str.c_str());
    char *ptr;
    ptr = strtok(cStr, delimiters);

    while (ptr != NULL) {
        container.push_back(ptr);
        ptr = strtok(NULL, delimiters);
    }
    delete[] cStr;
    return;
}

char* stripFlag(char *flag)
{
    char *strippedFlagStart = NULL;
    if (flag[0] != '-') { // not a flag
        return flag;
    }
    ;
    if ((flag[0] == '-') && (flag[1] == '-')) {
        strippedFlagStart = &flag[2];
    } else {
        strippedFlagStart = &flag[1];
    }
    return strippedFlagStart;
}

int countArgs(string args)
{
    if (args.size() == 0) {
        return 0;
    }
    int count = 1;
    for (string::iterator it = args.begin(); it < args.end(); ++it) {
        if (*it == ',') {
            count++;
        }
    }
    return count;
}

bool verifyNumOfArgs(string name, string value)
{
    //HACK : we don't check device numOfArgs because image device format might contain ","
    if ((name == "device") || (name == "d") ) {
        return true;
    }
    if ((name == "image") || (name == "i") ) {
        return true;
    }
    // Hack : VSD can be empty or contain "," so we shouldnt count its args
    if (name == "vsd") {
        return true;
    }

    int expected = FlagMetaData().getNumOfArgs(name);
    if (expected < 0) {
        printf(FLINT_INVALID_FLAG_ERROR, name.c_str());
        return false;
    }
    //HACK : flash_params is 3 argument but given with comma separated instead of spaces like the rest
    // so flag_arg_num gives a wrong value

    if (name == "flash_params") {
        expected = 3;
    }
    int actual = countArgs(value);
    if (actual < expected) {
        printf(FLINT_TOO_FEW_ARGS_ERROR, expected, actual);
        return false;
    } else if (actual > expected) {
        printf(FLINT_TOO_MANY_ARGS_ERROR, expected, actual);
        return false;
    }
    return true;
}

bool strToNum(string str, u_int64_t& num, int base = 0)
{
    char *endp;
    char *numStr = strcpy(new char[str.size() + 1], str.c_str());
    num = strtoul(numStr, &endp, base);
    if (*endp) {
        delete[] numStr;
        return false;
    }
    delete[] numStr;
    return true;
}

//this function is used in extracting both guids and macs from user
bool getGUIDFromStr(string str, guid_t& guid, string prefixErr = "")
{
    char *endp;
    u_int64_t g;
    g = strtoull(str.c_str(), &endp, 16);
    if (*endp || (g == 0xffffffffffffffffULL && errno == ERANGE)) {
        if (prefixErr.size() == 0) {
            printf("-E- Invalid Guid/Mac/Uid syntax (%s) %s \n", str.c_str(), errno ? strerror(errno) : "" );
        } else {
            printf("%s\n", prefixErr.c_str());
        }
        return false;
    }
    guid.h = (u_int32_t)(g >> 32);
    guid.l = (u_int32_t)(g & 0xffffffff);
    return true;
}

guid_t incrGuid(guid_t baseGuid, unsigned int incrVal)
{
    u_int64_t g = baseGuid.h;
    g = (g << 32) | baseGuid.l;
    g += incrVal;
    guid_t userGuid;
    userGuid.h = (u_int32_t)(g >> 32);
    userGuid.l = (u_int32_t)g;
    return userGuid;
}

guid_t GetBaseMac(guid_t baseGuid)
{
    guid_t baseMac;
    baseMac.l = (baseGuid.l & 0x00ffffff) | ((baseGuid.h & 0xff00) << 16);
    baseMac.h = baseGuid.h >> 16;
    return baseMac;
}

bool parseFlashParams(string params, flash_params_t& fp)
{
    // Step 1 split by ","
    std::vector<std::string> paramVec;
    splitByDelimiters(paramVec, params, ",");
    if (paramVec.size() != 3) {
        return false;
    }
    // Step 2 extract params
    fp.type_name = strcpy(new char[paramVec[0].length() + 1], paramVec[0].c_str());
    if (!fp.type_name) {
        return false;
    }
    u_int64_t tmp;
    if (!strToNum(paramVec[1], tmp)) {
        delete[] fp.type_name;
        return false;
    }
    fp.log2size = (int)tmp;
    if (!strToNum(paramVec[2], tmp)) {
        delete[] fp.type_name;
        return false;
    }
    fp.num_of_flashes = (int)tmp;
    return true;
}

/************************************
* Implementation of the Command line
* Parsing part of the Flint class
************************************/

#define FLASH_LIST_SZ 256

void Flint::initCmdParser()
{
    bool isExternal = false;
#ifdef EXTERNAL
    isExternal = true;
#endif
    AddDescription("flint is a FW (firmware) burning and flash memory operations tool for Mellanox Infiniband HCAs,"
                   "Ethernet NIC cards, and switch devices.");

    AddOptions("device",
               'd',
               "<device>",
               "Device flash is connected to.\n"
               "Commands affected: all");

    AddOptions("image",
               'i',
               "<image>",
               "Binary image file.\n"
               "Commands affected: burn, verify");

    AddOptions("ir",
        ' ',
        "",
        "Commands affected: burn");

    AddOptions("image_reactivation",
        ' ',
        "",
        "Commands affected: burn");

    AddOptions("help",
               'h',
               "",
               "Prints this message and exits");

    AddOptions("hh",
               ' ',
               "",
               "Prints extended command help");

    AddOptions("yes",
               'y',
               "",
               "Non interactive mode - assume answer \"yes\" to all questions.\n"
               "Commands affected: all");

    AddOptions("no",
               ' ',
               "",
               "Non interactive mode - assume answer \"no\" to all questions.\n"
               "Commands affected: all");

    AddOptions("guid",
               ' ',
               "<GUID>",
               "GUID base value. 4 GUIDs are automatically assigned to the following values:\n\n"
               "guid   -> node GUID\n"
               "guid+1 -> port1\n"
               "guid+2 -> port2\n"
               "guid+3 -> system image GUID.\n\n"
               "Note: port2 guid will be assigned even for a single port HCA - The HCA ignores this value.\n\n"
               "Commands affected: burn, sg");

    AddOptions("guids",
               ' ',
               "<GUIDS...>",
               "4 GUIDs must be specified here.\n"
               "The specified GUIDs are assigned to the following fields, respectively:\n"
               "node, port1, port2 and system image GUID.\n\n"
               "Note: port2 guid must be specified even for a single port HCA - The HCA ignores this value.\n"
               "It can be set to 0x0.\n\n"
               "Commands affected: burn, sg");

    AddOptions("mac",
               ' ',
               "<MAC>",
               "MAC address base value. 2 MACs are automatically assigned to the following values:\n\n"
               "mac    -> port1\n"
               "mac+1  -> port2\n\n"
               "Commands affected: burn, sg");

    AddOptions("macs",
               ' ',
               "<MACs...>",
               "2 MACs must be specified here.\n"
               "The specified MACs are assigned to port1, port2, respectively.\n"
               "Commands affected: burn, sg\n\n"
               "Note: -mac/-macs flags are applicable only for Mellanox\n"
               "\tTechnologies ethernet products.");

    AddOptions("uid",
               ' ',
               "<UID>",
               "ConnectIB/SwitchIB only. Derive and set the device UIDs (GUIDs, MACs, WWNs).\n"
               "UIDs are derived from the given base UID according to Mellanox Methodology\n"
               "Commands affected: burn, sg");

    AddOptions("blank_guids",
               ' ',
               "",
               "Burn the image with blank GUIDs and MACs (where applicable). These values can be set later using the"
               " \"sg\" command (see details below).\n\n"
               "Commands affected: burn");

    AddOptions("clear_semaphore",
               ' ',
               "",
               "Force clear the flash semaphore on the device.\n"
               "No command is allowed when this flag is used.\n"
               "NOTE: May result in system instability or flash corruption if the device or another application is"
               " currently using the flash.\n"
               "Exercise caution.\n");

    AddOptions("qq",
               ' ',
               "",
               "Run a quick query. When specified, flint will not perform full image integrity checks during the query"
               " operation. This may shorten execution time when running over slow interfaces (e.g., I2C, MTUSB-1).\n"
               "Commands affected: query");
    AddOptions("low_cpu",
               ' ',
               "",
               "When specified, cpu usage will be reduced. Run time might be increased\n"
               "Commands affected: query");

    AddOptions("next_boot_fw_ver",
               ' ',
               "",
               "When specified, only next boot fw version is fetched\n"
               "Commands affected: query",
               true);

    AddOptions("flashed_version",
               ' ',
               "",
               "When specified, only flashed fw version is fetched\n"
               "Commands affected: query");

    AddOptions("nofs",
               ' ',
               "",
               "Burn image in a non failsafe manner.");

    AddOptions("allow_psid_change",
               ' ',
               "",
               "Allow burning a FW image with a different PSID (Parameter Set ID)than the one currently on flash. Note"
               " that changing a PSID may cause the device to malfunction. Use only if you know what you are doing", isExternal);

    AddOptions("allow_rom_change",
               ' ',
               "",
               "Allow burning/removing a ROM to/from FW image when product version is present.\n"
               "Use only if you know what you are doing");

    AddOptions("override_cache_replacement",
               ' ',
               "",
               "On SwitchX/ConnectIB devices:\n"
               "Allow accessing the flash even if the cache replacement mode is enabled.\n"
               "NOTE: This flag is intended for advanced users only.\n"
               "Running in this mode may cause the firmware to hang.\n");

    AddOptions("no_flash_verify",
               ' ',
               "",
               "Do not verify each write on the flash.");

    AddOptions("use_fw",
               ' ',
               "",
               "Flash access will be done using FW (ConnectX-3/ConnectX-3Pro only).");

    AddOptions("silent",
               's',
               "",
               "Do not print burn progress flyer.\n"
               "Commands affected: burn");

    AddOptions("vsd",
               ' ',
               "<string>",
               "Write this string, of up to 208 characters, to VSD when burn.");

    AddOptions("use_image_ps",
               ' ',
               "",
               "Burn vsd as appears in the given image - do not keep existing VSD on flash.\n"
               "Commands affected: burn");

    AddOptions("use_image_guids",
               ' ',
               "",
               "Burn (guids/macs) as appears in the given image.\n"
               "Commands affected: burn");

    AddOptions("use_image_rom",
               ' ',
               "",
               "Do not save the ROM which exists in the device.\n"
               "Commands affected: burn");

    AddOptions("use_dev_rom",
               ' ',
               "",
               "Save the ROM which exists in the device.\n"
               "Commands affected: burn");

    AddOptions("ignore_dev_data",
               ' ',
               "",
               "Do not attempt to take device data sections from device(sections will be taken from the image. FS3"
               " Only).\n"
               "Commands affected: burn");

    AddOptions("no_fw_ctrl",
               ' ',
               "",
               "Do not attempt to work with the FW Ctrl update commands");

    AddOptions("use_dev_img_info",
               ' ',
               "",
               "preserve select image info fields from the device upon FW update (FS3 Only).\n"
               "Commands affected: burn",
               true);

    AddOptions("dual_image",
               ' ',
               "",
               "Make the burn process burn two images on flash (previously default algorithm). Current default"
               " failsafe burn process burns a single image (in alternating locations).\n"
               "Commands affected: burn");

    AddOptions("striped_image",
               ' ',
               "",
               "Use this flag to indicate that the given image file is in a \"striped image\" format.\n"
               "Commands affected: query verify");

    AddOptions("banks",
               ' ',
               "<bank>",
               "Set the number of attached flash devices (banks)");

    AddOptions("log",
               ' ',
               "<log_file>",
               "Print the burning status to the specified log file");
    
    

    char flashList[FLASH_LIST_SZ] = {0};
    char flashParDesc[FLASH_LIST_SZ * 2];
    Flash::get_flash_list(flashList, FLASH_LIST_SZ);
    snprintf(flashParDesc, FLASH_LIST_SZ * 2, "Use the given parameters to access the flash instead of reading them from "
             "the flash.\n" \
             "Supported parameters:\n" \
             "Type: The type of the flash, such as:%s.\n" \
             "log2size: The log2 of the flash size." \
             "num_of_flashes: the number of the flashes connected to the device.",
             flashList);


    AddOptions("flash_params",
               ' ',
               "<type, log2size, num_of_flashes>", flashParDesc);

    AddOptions("version",
               'v',
               "",
               "Version info.");

    AddOptions("no_devid_check",
               ' ',
               "",
               "ignore device_id checks", true);

    AddOptions("skip_ci_req",
               ' ',
               "",
               "skip sending cache image request to driver(windows)", true);

    AddOptions("ocr",
               ' ',
               "",
               "another flag for override cache replacement", true);
    AddOptions("private_key",
               ' ',
               "<key_file>",
               "path to PEM formatted private key to be used by the sign command");

    AddOptions("key_uuid",
               ' ',
               "<uuid_file>",
               "UUID matching the given private key to be used by the sign command");
    AddOptions("private_key2",
               ' ',
               "<key_file>",
               "path to PEM formatted private key to be used by the sign command");
    AddOptions("hmac_key",
               ' ',
               "<hmac_key>",
               "path to file containing key (For FS4 image only).");

    AddOptions("key_uuid2",
               ' ',
               "<uuid_file>",
               "UUID matching the given private key to be used by the sign command");

    for (map_sub_cmd_t_to_subcommand::iterator it = _subcommands.begin(); it != _subcommands.end(); it++) {
        if (it->first == SC_ResetCfg) {
            // hidden command so "forget" mentioning it
            continue;
        }
        string str1 = it->second->getFlagL() + ((it->second->getFlagS() == "") ? ("  ") :
                                                                                 ("|")) + it->second->getFlagS() \
                      + " " + it->second->getParam();
        string str2 = it->second->getDesc();

        AddOptionalSectionData("COMMANDS SUMMARY", str1, str2);
    }

    AddOptionalSectionData("RETURN VALUES", "0", "Successful completion.");
    AddOptionalSectionData("RETURN VALUES", "1", "An error has occurred.");
    AddOptionalSectionData("RETURN VALUES", "7", "For burn command - FW already updated - burn was aborted.");


    for (map_sub_cmd_t_to_subcommand::iterator it = _subcommands.begin(); it != _subcommands.end(); it++) {
        if (it->first == SC_ResetCfg) {
            // hidden command so "forget" mentioning it
            continue;
        }
        string str =   "Name:\n"
                     "\t" + it->second->getName() + "\n"
                     + "Description:\n"
                     "\t" + it->second->getExtDesc() + "\n"
                     + "Command:\n"
                     "\t" + it->second->getFlagL() +
                     ((it->second->getFlagS() == "") ? ("  ") : ("|")) + it->second->getFlagS()
                     + " " + it->second->getParam() + "\n"
                     + "Parameters:\n"
                     "\t" + it->second->getParamExp() + "\n"
                     + "Examples:\n"
                     "\t" + it->second->getExample() + "\n\n\n";
        AddOptionalSectionData("COMMANDS DESCRIPTION", str);
    }


    _cmdParser.AddRequester(this);
}

ParseStatus Flint::HandleOption(string name, string value)
{
    //TODO: consider verifying num of args inside each if statements that needs its arg num verified
    //      thus we will be able to get rid of the hacks inside the function in the expense of a longer code.
    if (!(verifyNumOfArgs(name, value))) {
        return PARSE_ERROR;
    }
    int delta = 1;
    if (name == "device" || name == "d") {
        _flintParams.device_specified = true;
        _flintParams.device = value;
    } else if (name == "help" || name == "h") {
        vector<string> excluded_sections;
        excluded_sections.push_back("COMMANDS DESCRIPTION");
        cout << _cmdParser.GetUsage(false, excluded_sections);
        return PARSE_OK_WITH_EXIT;
    } else if (name == "version" || name == "v") {
#ifdef EXTERNAL
        print_version_string(FLINT_NAME, "");
#else
        print_version_string(FLINT_NAME "(oem)", "");
#endif
        return PARSE_OK_WITH_EXIT;
    } else if (name == "hh") {
        cout << _cmdParser.GetUsage();
        return PARSE_OK_WITH_EXIT;
    } else if (name == "no_devid_check") {
        _flintParams.no_devid_check = true;
    } else if (name == "skip_ci_req") {
        _flintParams.skip_ci_req = true;
    } else if (name == "guid") {
        _flintParams.guid_specified = true;
        guid_t g;
        if (!getGUIDFromStr(value, g)) {
            return PARSE_ERROR;
        }
        for (int i = 0; i < GUIDS; i++) {
            _flintParams.user_guids.push_back(incrGuid(g, i));
        }
        // for (std::vector<guid_t>::iterator it=_flintParams.user_guids.begin();it != _flintParams.user_guids.end(); it++){
        //     printf("%8.8x%8.8x\n",it->h,it->l);
        // }
    } else if (name == "guids") {
        _flintParams.guids_specified = true;
        std::vector<std::string> strs;
        splitByDelimiters(strs, value, ",");
        if (strs.size() != GUIDS) {
            return PARSE_ERROR;
        }
        for (int i = 0; i < GUIDS; i++) {
            guid_t g;
            if (!getGUIDFromStr(strs[i], g)) {
                return PARSE_ERROR;
            } else {
                _flintParams.user_guids.push_back(g);
            }
        }
    } else if (name == "mac") {
        _flintParams.mac_specified = true;
        guid_t m;
        if (!getGUIDFromStr(value, m)) {
            return PARSE_ERROR;
        }
        for (int i = 0; i < MACS; i++) {
            _flintParams.user_macs.push_back(incrGuid(m, i));
        }
    } else if (name == "macs") {
        _flintParams.macs_specified = true;
        std::vector<std::string> strs;
        splitByDelimiters(strs, value, ",");
        if (strs.size() != MACS) {
            return PARSE_ERROR;
        }
        for (int i = 0; i < MACS; i++) {
            guid_t m;
            if (!getGUIDFromStr(strs[i], m)) {
                return PARSE_ERROR;
            } else {
                _flintParams.user_macs.push_back(m);
            }
        }
    } else if (name == "uid") {
        _flintParams.uid_specified = true;
        if (!getGUIDFromStr(value, _flintParams.baseUid)) {
            return PARSE_ERROR;
        }
    } else if (name == "blank_guids") {
        _flintParams.blank_guids = true;
    } else if (name == "clear_semaphore") {
        _flintParams.clear_semaphore = true;
    } else if (name == "image" || name == "i") {
        _flintParams.image_specified = true;
        _flintParams.image = value;
    } else if (name == "qq") {
        _flintParams.quick_query = true;
        _flintParams.skip_rom_query = true;
        delta = 2;
    } else if (name == "low_cpu") {
        _flintParams.low_cpu = true;
    } else if (name == "next_boot_fw_ver" || name == "flashed_version") {
        _flintParams.next_boot_fw_ver = true;
    } else if (name == "nofs") {
        _flintParams.nofs = true;
    } else if (name == "allow_psid_change") {
        _flintParams.allow_psid_change = true;
    } else if (name == "allow_rom_change") {
        _flintParams.allow_rom_change = true;
    } else if (name == "override_cache_replacement" || name == "ocr") {
        _flintParams.override_cache_replacement = true;
    } else if (name == "use_fw") {
        _flintParams.use_fw = true;
    } else if (name == "no_flash_verify") {
        _flintParams.no_flash_verify = true;
    } else if (name == "silent" || name == "s") {
        _flintParams.silent = true;
    } else if (name == "yes" || name == "y") {
        _flintParams.yes = true;
    } else if (name == "no") {
        _flintParams.no = true;
    } else if (name == "vsd") {
        _flintParams.vsd_specified = true;
        _flintParams.vsd = value;
    } else if (name == "use_image_ps") {
        _flintParams.use_image_ps = true;
    } else if (name == "use_image_guids") {
        _flintParams.use_image_guids = true;
    } else if (name == "use_image_rom") {
        _flintParams.use_image_rom = true;
    } else if (name == "use_dev_rom") {
        _flintParams.use_dev_rom = true;
    } else if (name == "ignore_dev_data") {
        _flintParams.ignore_dev_data = true;
    } else if (name == "no_fw_ctrl") {
        _flintParams.no_fw_ctrl = true;
    } else if (name == "dual_image") {
        _flintParams.dual_image = true;
    } else if (name == "striped_image") {
        _flintParams.striped_image = true;
    } else if (name == "use_dev_img_info") {
        _flintParams.use_dev_img_info = true;
    } else if (name == "ir" || name == "image_reactivation") {
        _flintParams.image_reactivation = true;
    }
    else if (name == "banks") {
        _flintParams.banks_specified = true;
        u_int64_t banksNum;
        if (!strToNum(value, banksNum)) {
            return PARSE_ERROR;
        }
        _flintParams.banks = (int)banksNum;
    } else if (name == "log") {
        _flintParams.log_specified = true;
        _flintParams.log = value;
    } else if (name == "flash_params") {
        _flintParams.flash_params_specified = true;
        if (!parseFlashParams(value, _flintParams.flash_params)) {
            return PARSE_ERROR;
        }
        //printf("-D- flashType=%s , log2size = %d , numOfBanks = %d\n", _flintParams.flash_params.type_name, _flintParams.flash_params.log2size, _flintParams.flash_params.num_of_flashes);
    } else if (name == "private_key") {
        _flintParams.privkey_specified = true;
        _flintParams.privkey_file = value;
    } else if (name == "key_uuid") {
        _flintParams.uuid_specified = true;
        _flintParams.privkey_uuid = value;
    } else if (name == "hmac_key") {
        _flintParams.key_specified = true;
        _flintParams.key = value;
    } else if (name == "private_key2") {
        _flintParams.privkey2_specified = true;
        _flintParams.privkey2_file = value;
    } else if (name == "key_uuid2") {
        _flintParams.uuid2_specified = true;
        _flintParams.privkey2_uuid = value;
    } else {
        cout << "Unknown Flag: " << name;
        cout << _cmdParser.GetSynopsis();
        return PARSE_ERROR;
    }
    _flintParams.num_of_args += delta;
    return PARSE_OK;
}

#define IS_NUM(cha) (((cha) >= '0') && ((cha) <= '9'))

ParseStatus Flint::parseCmdLine(int argc, char *argv[])
{
    //Step1 separate between option section and cmd section
    SubCmdMetaData subCmds;
    FlagMetaData flags;
    char **argvCmd = NULL;
    char **argvOpt = NULL;
    int argcCmd = 0, argcOpt = 0;
    bool foundOptionWhenLookingForCmd = false;
    ParseStatus rc;
    char **newArgv = NULL;
    int newArgc = 0;
    int i = 1, j = 1, argStart = 1, argEnd = 1;

    for (int k = (argc - 1); k > 0; k--) {
        if ((subCmds.getCmdNum(argv[k])) != SC_No_Cmd) { // i.e we found a subcommand
            if (foundOptionWhenLookingForCmd) {
                cout << "Specifying options flags after command is not allowed.\n\n";
                rc = PARSE_ERROR_SHOW_USAGE;
                goto clean_up;
            }
            argcOpt = k;
            argvOpt = argv;
            argcCmd = argc - k;
            argvCmd = &argv[k];
        } else if (argv[k][0] == '-' && !IS_NUM(argv[k][1])) {
            foundOptionWhenLookingForCmd = true;
        }
    }
    if (argcCmd == 0) {
        //no subcommand found
        argcOpt = argc;
        argvOpt = argv;
    }
    //printf("-D- argcOpt:%d argvOpt:%s argcCmd:%d argvCmd:%s\n", argcOpt, argvOpt[0], argcCmd, argvCmd[0]);
    //_cmdparser should deal with the case of no arguments in argv except the program.
    //Step2 unite with comma multiple args in the options section
    newArgv = new char*[argcOpt];
    //first arg is the flint command we can copy as is
    newArgv[0] = strcpy(new char[strlen(argvOpt[0]) + 1], argvOpt[0]);
    while (i < argcOpt) {
        if (argvOpt[i][0] == '-' && !IS_NUM(argvOpt[i][1])) { //its a flag (and not a number) so we copy to new_argv as is
            newArgv[j] = strcpy(new char[strlen(argvOpt[i]) + 1],
                                argvOpt[i]);
            i++;
            j++;
        } else { //its an argument
            //find next flag if exsists
            argStart = i;
            argEnd = i;
            int argsSize = 0;
            while (argEnd < argcOpt && (argvOpt[argEnd][0] != '-' || IS_NUM(argvOpt[argEnd][1]))) {
                argsSize += strlen(argvOpt[argEnd]) + 1; //for the comma
                argEnd++;
            }
            i = argEnd;
            //concatenate all the args with comma between them to a single string
            newArgv[j] = new char[argsSize + 1];
            newArgv[j][0] = '\0';
            while (argStart < argEnd) {
                if (argStart == argEnd - 1) {
                    //no need to add comma to the last arg
                    strcat(newArgv[j], argvOpt[argStart]);
                } else {
                    strcat(newArgv[j], argvOpt[argStart]);
                    strcat(newArgv[j], ",");
                }
                argStart++;
            }
            j++;
        }
    }
    newArgc = j;

    //Step3 set the command and its args in the FlintParams struct if present
    if (argcCmd > 0) {
        this->_flintParams.cmd = subCmds.getCmdNum(argvCmd[0]);
        for (int i = 1; i < argcCmd; ++i) {
            this->_flintParams.cmd_params.push_back(string(argvCmd[i]));
        }
    } else { //no command found deal with either missing/invalid command
        //find last flag
        int lastFlagPos;
        char *strippedFlag;
        for (lastFlagPos = argc - 1; lastFlagPos > 0; lastFlagPos--) {
            if (argv[lastFlagPos][0] == '-' && !IS_NUM(argv[lastFlagPos][1])) {
                break;
            }
        }
        if (lastFlagPos == 0) {
            cout << FLINT_NO_OPTIONS_FOUND_ERROR << endl;
            rc = PARSE_ERROR;
            goto clean_up;
        }
        strippedFlag = stripFlag(argv[lastFlagPos]);
        //check how many args it needs
        int numOfArgs = flags.getNumOfArgs(strippedFlag);
        //if too many args return arg in pos num_of_args+1 as the invalid command.
        if ((argc - 1 - lastFlagPos) > numOfArgs) {
            printf(FLINT_INVALID_COMMAD_ERROR, argv[argc - 1]);
            rc = PARSE_ERROR_SHOW_USAGE;
            goto clean_up;
        }
    }
    //Step5 parse option section using _cmdParser
    rc = this->_cmdParser.ParseOptions(newArgc, newArgv);
    // Step 6 Delete allocated memory
clean_up: for (int i = 0; i < newArgc; i++) {
        delete[] newArgv[i];
    }
    delete[] newArgv;
    if (rc == PARSE_ERROR_SHOW_USAGE) {
        cout << _cmdParser.GetSynopsis();
    }
    return rc;
}
