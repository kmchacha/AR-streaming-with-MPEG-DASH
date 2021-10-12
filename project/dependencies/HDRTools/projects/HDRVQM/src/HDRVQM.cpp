/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * <OWNER> = Samsung Electronics, Apple Inc.
 * <ORGANIZATION> = Samsung Electronics, Apple Inc.
 * <YEAR> = 2016
 *
 * Copyright (c) 2016, Samsung Electronics, Apple Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *  * Neither the name of the <ORGANIZATION> nor the names of its contributors may
 *    be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

/*!
 *************************************************************************************
 * \file HDRVQM.cpp
 *
 * \brief
 *    HDRVQM main project class
 *
 * \author
 *     - Kulbhushan Pachauri             <kb.pachauri@samsung.com>
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *************************************************************************************
 */


//-----------------------------------------------------------------------------
// Include headers
//-----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Global.H"
#include "ProjectParameters.H"
#include "Parameters.H"
#include "HDRVQM.H"
#include "HDRVQMFrame.H"

//-----------------------------------------------------------------------------
// Local functions
//-----------------------------------------------------------------------------

/*!
 ***********************************************************************
 * \brief
 *   print help message and exit
 ***********************************************************************
 */
void HDRVQMExit (char *func_name) {
    printf("Usage: %s [-h] {[-s] [-m]} [-f config.cfg] "
    "{[-p Param1=Value1]..[-p ParamM=ValueM]}\n\n"

    "Options:\n"
    "   -h :  Help mode\n"
    "   -s :  Silent mode\n"
    "   -f :  Read <config.cfg> for reseting selected parameters.\n"
    "   -p :  Set parameter <ParamM> to <ValueM>.\n"
    "         See default config.cfg file for description of all parameters.\n\n"
    
    "## Supported video file formats\n"
    "   RAW:  .yuv -> YUV 4:2:0\n\n"
    
    "## Examples of usage:\n"
    "   %s\n"
    "   %s  -h\n"
    "   %s  -f config.cfg\n"
    "   %s  -f config.cfg -p SourceFile=\"seq.yuv\" -p width=176 -p height=144\n" 
    ,func_name,func_name,func_name,func_name,func_name);
    exit(EXIT_FAILURE);
}

HDRVQM *HDRVQM::create(ProjectParameters *inputParams) {
  HDRVQM *result = NULL;
    result = new HDRVQMFrame(inputParams);
  return result;
}


//-----------------------------------------------------------------------------
// Main function
//-----------------------------------------------------------------------------

int main(int argc, char **argv) {
  using ::hdrtoolslib::params;
  using ::hdrtoolslib::ZERO;
  int help_mode = false;
  int a;
  HDRVQM* hdrProcess;
  int numCLParams = 0, par;
  char **cl_params = new char* [hdrtoolslib::MAX_CL_PARAMS];
  char *parfile= new char[hdrtoolslib::FILE_NAME_SIZE];
  bool readConfig = false;
  
  params = &ccParams;
  params->refresh();
  
  strcpy(parfile, DEFAULTCONFIGFILENAME );
  for ( par = 0; par < hdrtoolslib::MAX_CL_PARAMS; par++ ) {
    cl_params[par] = new char [hdrtoolslib::MAX_CL_PARAM_LENGTH];
    //Lets reset this to make sure no garbage remains
    memset(cl_params[par],0, hdrtoolslib::MAX_CL_PARAM_LENGTH * sizeof(char));
  }
  
  // start here
  params->m_silentMode = false;

  for (a = 1; a < argc; a++) {
    if (argv[a][ZERO] == '-') {
      if (strcasecmp(argv[a], "-h") == ZERO)
        help_mode = true;
      else if (strcasecmp (argv[a], "-v") == ZERO) {
        printf("%s ",argv[ZERO]);
        printf("V." VERSION ": compiled " __DATE__ " " __TIME__ "\n");
        
        exit(EXIT_FAILURE);
      }
      else if (strcasecmp(argv[a], "-s") == ZERO)
        params->m_silentMode = true;
      else if (strcasecmp(argv[a], "-p") == ZERO) {
        // copy parameter to buffer
        if ( (a + 1) < argc ) {
          //strncpy( cl_params[ numCLParams ], argv[a + 1],strlen(argv[a + 1]));
          snprintf(cl_params[ numCLParams ], hdrtoolslib::MAX_CL_PARAM_LENGTH, "%s", argv[a + 1]);
          numCLParams++;
          // jump ahead or else the loop will break and the cfg filename will not be reached
          // since it assumes it always finds "-"
          a++;
        }
        else {
          exit(EXIT_FAILURE);
        }
      }
      else if (strcasecmp(argv[a], "-f") == ZERO) {
        // input parameter file
        if ( (a + 1) < argc ) {
          strcpy(parfile, argv[a + 1] );
          printf("Parsing configuration file %s.\n", parfile);
          params->readConfigFile(parfile);
          readConfig = true;
          a++;
        }
        else {
          exit(EXIT_FAILURE);
        }
      }
      else
        help_mode = true;
    }
    else
      break;
  }
  
  if (help_mode == true)
    params->m_silentMode = false;
  
  if (params->m_silentMode == false) {
    printf("---------------------------------------------------------\n");
    printf(" HDR VQM tool - Version %s (%s)\n",HDR_VQM_VERSION,VERSION);
    printf("---------------------------------------------------------\n");
  }
  
  if (help_mode == true) {
    HDRVQMExit(argv[ZERO]);
  }
  
  // Prepare parameters
  params->configure(parfile, cl_params, numCLParams, readConfig );
  
  hdrProcess = HDRVQM::create((ProjectParameters *) params);
  
  hdrProcess->init         ((ProjectParameters *) params);
  hdrProcess->outputHeader ((ProjectParameters *) params);
  hdrProcess->process      ((ProjectParameters *) params);
  hdrProcess->outputFooter ((ProjectParameters *) params);
  hdrProcess->destroy();
  
  delete hdrProcess;
  
  for (par = 0; par < hdrtoolslib::MAX_CL_PARAMS; par++){
    delete [] cl_params[par];
  }
  
  delete [] cl_params;
  delete [] parfile;
  
  return EXIT_SUCCESS;  
}

//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
