/*==============================================================================
Copyright(c) 2017 Intel Corporation

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files(the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and / or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
============================================================================*/

#pragma once

#ifdef __cplusplus
#if GMM_LOG_AVAILABLE

#include "spdlog/spdlog.h"

/////////////////////////////////////////////////////////////////////////////////////
/// @file Logger.h
/// @brief This file contains the functions that are useful for logging in GmmLib
/////////////////////////////////////////////////////////////////////////////////////
namespace GmmLib
{
    class Logger
    {
        private:
            /// This enum determines where the log goes
            enum GmmLogMethod
            {
                ToOSLog,                                        ///< Log is sent to OS logging infrastructure (Debugger on Windows)
                ToFile,                                         ///< Log is written to file
            } LogMethod;                                        ///< Indicates which Logging method is preferred

            spdlog::level::level_enum       LogLevel;           ///< Indicates the max log level to print

        private:
            Logger();
            ~Logger();

            bool            GmmLogInit();

        public:
            std::shared_ptr<spdlog::logger> SpdLogger;          ///< spdlog instance

            /////////////////////////////////////////////////////////////////////////////////////
            /// Creates a Logger singlton per process
            ///
            /// @remark
            ///     If there are multiple modules loaded in one process, this singleton will be
            ///     per module. For example: D3D10 and D3D12 are all used in process A. Both D3D10
            ///     and D3D12 are built with GmmLib.lib. They are considered to be two modules and
            ///     each has a Logger instance (singleton).
            ///
            /// @return Logger instance
            /////////////////////////////////////////////////////////////////////////////////////
            static Logger& CreateGmmLogSingleton()
            {
                static Logger GmmLoggerPerProc;
                return GmmLoggerPerProc;
            }
    };
} // namespace GmmLib

// Macros
extern GmmLib::Logger& GmmLoggerPerProc;
#endif
#endif