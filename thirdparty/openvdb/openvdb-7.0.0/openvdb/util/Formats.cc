// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

#include "Formats.h"
#include <openvdb/Platform.h>
#include <iostream>
#include <iomanip>


namespace openvdb {
OPENVDB_USE_VERSION_NAMESPACE
namespace OPENVDB_VERSION_NAME {
namespace util {

int
printBytes(std::ostream& os, uint64_t bytes,
    const std::string& head, const std::string& tail,
    bool exact, int width, int precision)
{
    const uint64_t one = 1;
    int group = 0;

    // Write to a string stream so that I/O manipulators like
    // std::setprecision() don't alter the output stream.
    std::ostringstream ostr;
    ostr << head;
    ostr << std::setprecision(precision) << std::setiosflags(std::ios::fixed);
    if (bytes >> 40) {
        ostr << std::setw(width) << (double(bytes) / double(one << 40)) << " TB";
        group = 4;
    } else if (bytes >> 30) {
        ostr << std::setw(width) << (double(bytes) / double(one << 30)) << " GB";
        group = 3;
    } else if (bytes >> 20) {
        ostr << std::setw(width) << (double(bytes) / double(one << 20)) << " MB";
        group = 2;
    } else if (bytes >> 10) {
        ostr << std::setw(width) << (double(bytes) / double(one << 10)) << " KB";
        group = 1;
    } else {
        ostr << std::setw(width) << bytes << " Bytes";
    }
    if (exact && group) ostr << " (" << bytes << " Bytes)";
    ostr << tail;

    os << ostr.str();

    return group;
}


int
printNumber(std::ostream& os, uint64_t number,
    const std::string& head, const std::string& tail,
    bool exact, int width, int precision)
{
    int group = 0;

    // Write to a string stream so that I/O manipulators like
    // std::setprecision() don't alter the output stream.
    std::ostringstream ostr;
    ostr << head;
    ostr << std::setprecision(precision) << std::setiosflags(std::ios::fixed);
    if (number / UINT64_C(1000000000000)) {
        ostr << std::setw(width) << (double(number) / 1000000000000.0) << " trillion";
        group = 4;
    } else if (number / UINT64_C(1000000000)) {
        ostr << std::setw(width) << (double(number) / 1000000000.0) << " billion";
        group = 3;
    } else if (number / UINT64_C(1000000)) {
        ostr << std::setw(width) << (double(number) / 1000000.0) << " million";
        group = 2;
    } else if (number / UINT64_C(1000)) {
        ostr << std::setw(width) << (double(number) / 1000.0) << " thousand";
        group = 1;
    } else {
        ostr << std::setw(width) << number;
    }
    if (exact && group) ostr << " (" << number << ")";
    ostr << tail;

    os << ostr.str();

    return group;
}

int
printTime(std::ostream& os, double milliseconds,
  const std::string& head, const std::string& tail,
  int width, int precision, int verbose)
  {
    int group = 0;

    // Write to a string stream so that I/O manipulators like
    // std::setprecision() don't alter the output stream.
    std::ostringstream ostr;
    ostr << head;
    ostr << std::setprecision(precision) << std::setiosflags(std::ios::fixed);

    if (milliseconds >= 1000.0) {// one second or longer
      const uint32_t seconds = static_cast<uint32_t>(milliseconds / 1000.0) % 60 ;
      const uint32_t minutes = static_cast<uint32_t>(milliseconds / (1000.0*60)) % 60;
      const uint32_t hours   = static_cast<uint32_t>(milliseconds / (1000.0*60*60)) % 24;
      const uint32_t days    = static_cast<uint32_t>(milliseconds / (1000.0*60*60*24));
      if (days>0) {
        ostr << days << (verbose==0 ? "d " : days>1 ? " days, " : " day, ");
        group = 4;
      }
      if (hours>0) {
        ostr << hours << (verbose==0 ? "h " : hours>1 ? " hours, " : " hour, ");
        if (!group) group = 3;
      }
      if (minutes>0) {
        ostr << minutes << (verbose==0 ? "m " : minutes>1 ? " minutes, " : " minute, ");
        if (!group) group = 2;
      }
      if (seconds>0) {
        if (verbose) {
          ostr << seconds << (seconds>1 ? " seconds and " : " second and ");
          const double msec = milliseconds - (seconds + (minutes + (hours + days * 24) * 60) * 60) * 1000.0;
          ostr << std::setw(width) << msec << " milliseconds (" << milliseconds << "ms)";
        } else {
          const double sec = milliseconds/1000.0 - (minutes + (hours + days * 24) * 60) * 60;
          ostr << std::setw(width) << sec << "s";
        }
      } else {// zero seconds
        const double msec = milliseconds - (minutes + (hours + days * 24) * 60) * 60 * 1000.0;
        if (verbose) {
          ostr << std::setw(width) << msec << " milliseconds (" << milliseconds << "ms)";
        } else {
          ostr << std::setw(width) << msec << "ms";
        }
      }
      if (!group) group = 1;
    } else {// less than a second
      ostr << std::setw(width) << milliseconds << (verbose ? " milliseconds" : "ms");
    }

    ostr << tail;

    os << ostr.str();

    return group;
  }

} // namespace util
} // namespace OPENVDB_VERSION_NAME
} // namespace openvdb
