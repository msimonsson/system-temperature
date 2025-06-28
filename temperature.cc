// Copyright (c) 2022 Mikael Simonsson <https://mikaelsimonsson.com>.
// SPDX-License-Identifier: BSL-1.0

#include "snn-core/main.hh"
#include "snn-core/null_term.hh"
#include "snn-core/optional.hh"
#include "snn-core/strcore.hh"
#include "snn-core/vec.hh"
#include "snn-core/algo/max.hh"
#include "snn-core/env/options.hh"
#include "snn-core/file/standard/error.hh"
#include "snn-core/file/standard/out.hh"
#include "snn-core/fmt/print.hh"
#include "snn-core/fmt/fn/format.hh"
#include "snn-core/fn/common.hh"
#include "snn-core/pair/common.hh"
#include "snn-core/range/step.hh"
#include "snn-core/range/view/element.hh"
#include <sys/sysctl.h> // sysctlbyname

namespace snn::app
{
    class temperature final
    {
      public:
        explicit temperature(const bool show_all) noexcept
            : show_all_{show_all}
        {
        }

        [[nodiscard]] int print() const
        {
            vec<pair::name_value<str, int>> temperatures;

            // Get temperatures from sysctl: hw.acpi.thermal.tzN.temperature
            // Is the count available somewhere? Assume no more than X.

            for (const auto i : range::step{0, 4})
            {
                str name;
                name << "hw.acpi.thermal.tz" << as_num(i) << ".temperature";
                if (auto kelvin_maybe = sysctl_by_name_(name))
                {
                    temperatures.append_inplace(std::move(name),
                                                kelvin_to_celsius_(kelvin_maybe.value()));
                }
                else
                {
                    break;
                }
            }

            // Get temperatures from sysctl: dev.cpu.N.temperature

            const int cpu_count = sysctl_by_name_("hw.ncpu").value_or(-1);
            if (cpu_count > 0)
            {
                for (const auto i : range::step{0, cpu_count})
                {
                    str name;
                    name << "dev.cpu." << as_num(i) << ".temperature";
                    if (auto kelvin_maybe = sysctl_by_name_(name))
                    {
                        temperatures.append_inplace(std::move(name),
                                                    kelvin_to_celsius_(kelvin_maybe.value()));
                    }
                    else
                    {
                        break;
                    }
                }
            }

            if (show_all_)
            {
                strbuf formatted{init::reserve, 500};
                temperatures.each(fn::unpack{fmt::fn::format{"{}: {}°\n", formatted}});
                file::standard::out{} << formatted;
            }
            else
            {
                const int highest_temperature =
                    algo::max(temperatures.range() | range::v::element<1>{}).value_or(0);
                fmt::print("{}°\n", highest_temperature);
            }

            return temperatures ? constant::exit::success : constant::exit::failure;
        }

      private:
        bool show_all_;

        static constexpr int kelvin_to_celsius_(const int kelvin) noexcept
        {
            // From FreeBSD source: /usr/src/sys/dev/coretemp/coretemp.c
            // #define TZ_ZEROC 2731
            // val = (...) * 10 + TZ_ZEROC;
            return (kelvin - 2731 + 5) / 10; // Add 5 to round up.
        }

        static optional<int> sysctl_by_name_(const null_term<const char*> name) noexcept
        {
            int value        = 0;
            usize value_size = sizeof(value);
            if (::sysctlbyname(name.get(), &value, &value_size, nullptr, 0) == 0)
            {
                return value;
            }
            return nullopt;
        }
    };
}

namespace snn
{
    int main(const array_view<const env::argument> arguments)
    {
        env::options opts{arguments,
                          {
                              {"all", 'a'},
                              {"help", 'h'},
                          },
                          assume::is_sorted};

        if (!opts)
        {
            fmt::print_error_line("Error: {}.", opts.error_message());
            return constant::exit::failure;
        }

        const bool show_all  = opts.option('a').is_set();
        const bool show_help = opts.option('h').is_set();

        if (show_help)
        {
            strbuf usage{init::reserve, 250};

            usage << "Write system temperature (CPU/ACPI) to stdout.\n\n"
                  << "Only the highest temperature is written,"
                  << " or 0 if no temperature is available.\n\n";

            usage << "Usage: " << opts.program_name() << " [-a|-h]\n\n";

            usage << "Options:\n"
                  << "-a --all   List all values.\n"
                  << "-h --help  Show this help.\n";

            file::standard::error{} << usage;

            return constant::exit::failure;
        }

        return app::temperature{show_all}.print();
    }
}
