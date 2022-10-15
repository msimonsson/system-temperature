# System temperature (FreeBSD)

Write system temperature to `stdout`.

Can be used by itself or with a status bar (like [Polybar](https://github.com/polybar/polybar)).

_This also serves as a sample application for the [snn-core][snncore] library and can be built with the [build-tool][buildtool]._


## Usage

```console
$ temperature
35°
```

```console
$ temperature --all
dev.cpu.0.temperature: 33°
dev.cpu.1.temperature: 33°
dev.cpu.2.temperature: 33°
dev.cpu.3.temperature: 33°
dev.cpu.4.temperature: 35°
dev.cpu.5.temperature: 35°
dev.cpu.6.temperature: 31°
dev.cpu.7.temperature: 31°
dev.cpu.8.temperature: 34°
dev.cpu.9.temperature: 34°
dev.cpu.10.temperature: 31°
dev.cpu.11.temperature: 31°
```

```console
$ temperature --help
Write system temperature (CPU/ACPI) to stdout.

Only the highest temperature is written, or 0 if no temperature is available.

Usage: ./temperature [-a|-h]

Options:
-a --all   List all values.
-h --help  Show this help.
```


## Temperature sources

 * `man coretemp`
 * `man acpi_thermal`


## License

See [LICENSE](LICENSE). Copyright © 2022 [Mikael Simonsson](https://mikaelsimonsson.com).


[buildtool]: https://github.com/snncpp/build-tool
[snncore]: https://github.com/snncpp/snn-core
