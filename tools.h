#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <latch>

#include <chrono>
using namespace std::chrono_literals;
using result_unit = typename std::chrono::nanoseconds::rep;

constexpr int contention_thread_count = 3;

struct console_cursor_disabler{
   console_cursor_disabler() { std::cout << "\x1b[?25l";}
   ~console_cursor_disabler(){ std::cout << "\x1b[?25h";}
};


[[nodiscard]] inline auto get_percentile(
   const std::vector<double>& vec,
   const double percentile
) -> double
{
   std::vector<double> sorted = vec;
   std::ranges::sort(sorted);
   const int perc_index = static_cast<int>(percentile / 100.0 * std::size(sorted));
   return sorted[perc_index];
}


inline auto goto_horizontal(int pos) -> std::string
{
   std::string msg = "\x1b[";
   msg += std::to_string(pos);
   msg += "G";
   return msg;
}

template<typename fun_type>
auto just_do_it(
   const int n,
   const std::string& description,
   const fun_type& get_measurement
) -> void
{
   std::cout << description << ":";
   const int progress_pos = static_cast<int>(description.size()) + 3;
   std::vector<result_unit> runtimes;
   runtimes.reserve(n);
   {
      console_cursor_disabler no_cursor;
      for (int i = 0; i < n; ++i) {
         if (i % 100 == 0) {
            const int percentage = 100 * i / n;
            std::cout << goto_horizontal(progress_pos) << percentage << "%    ";
         }
         const auto measurement = get_measurement();

         // Skip the first data point
         if(i==0)
            continue;

         runtimes.emplace_back(measurement);
      }
   }

   // Result writing
   std::string filename = "python/";
   filename += description;
   filename += ".txt";

   std::ofstream file_writer(filename);
   for (const result_unit value : runtimes)
      file_writer << std::to_string(value) << "\n";
   std::cout << goto_horizontal(progress_pos) << "done\n";
}
