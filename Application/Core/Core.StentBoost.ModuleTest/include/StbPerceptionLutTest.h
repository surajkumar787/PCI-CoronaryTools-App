// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <gtest/gtest.h>
#include <StbParameters.h>
#include <fstream>
#include "PathUtil.h"

// #define DEBUG_SAVE_LUT_AS_TEXT

class StbPerceptionLutTest : public ::testing::Test
{
    
};

TEST_F(StbPerceptionLutTest, GivenPerceptionLutFromIQTeam_WhenLutIsConvertedToIniFile_ThenValuesAreTheSame)
{
    // Load LUT via Settings ini (stb.ini) does not contain perception lut by default.
	Pci::Core::StbParameters stbParams( L"stb.ini", L"settings.ini");
    
    // Load text file from IQ team
    const int inputRange       = 1<<14;
    const int inputRangeOffset = 1<<15;
    std::vector<unsigned short> iqLut;
    iqLut.reserve(inputRange);
    const std::string iqFile( CommonPlatform::Utilities::AbsolutePathFromExeDir("PerceptionLUT.txt"));

    std::string line;
    std::ifstream file (iqFile);
    ASSERT_TRUE( file.is_open());
    
    while ( getline (file,line) )
    {
        if ( !line.empty() && line[0] != ';')
        {
            unsigned short y = static_cast<unsigned short>(std::stoi(line));
            iqLut.push_back(y);
        }
    }
    file.close();

#ifdef DEBUG_SAVE_LUT_AS_TEXT
    const std::string tmpfile("StbPerceptionLutTest_RawPerceptionLut.txt");
    remove(tmpfile.c_str());
    std::ofstream myfile;
    myfile.open(tmpfile);
#endif

    for ( int index = 0 ; index < stbParams.perceptionLut->lutSize ;index++)
    {
        int input = index - inputRangeOffset; 
        int output  = stbParams.perceptionLut->lut.pointer[index];

#ifdef DEBUG_SAVE_LUT_AS_TEXT
        // Dump full lut
        myfile << index << "," << input << "," << output << std::endl;
#endif

        if ( input < 0 )
        {
            // Clip negative values to output 0.
            ASSERT_EQ( 0, output);
        }
        else if ( input < inputRange)
        {
            // Actual comparison with values from IQ files values.
            ASSERT_EQ( iqLut[input], output );
        }
        else
        {
            // Clip values above input range to maximum output
            ASSERT_EQ( stbParams.perceptionLut->maxValue, output);
        }
    }

#ifdef DEBUG_SAVE_LUT_AS_TEXT
    myfile.close();
#endif

}


