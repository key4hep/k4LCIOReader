# k4LCIOReader

Generate EDM4hep collections from LCIO format data.

`k4LCIOReader` is a subclass that inherits from `podio::IReader`. And it holds an instance of `LCIO::LCReader`.

![k4LCIOReader](k4LCIOReader.png)

Some converters (from LCCollection to EDM4hep collection) are still being in developing.

There is also an algorithm wrapper [LCIOInput](https://github.com/ihep-sft-group/LCIOInput), by which we can read LCIO with key4hep/k4FWCore in Gaudi.

## Dependencies

- [LCIO](https://github.com/iLCSoft/LCIO)
- [podio](https://github.com/AIDASoft/podio)
- [EDM4hep](https://github.com/key4hep/EDM4hep)

## Build

Suppose environment has been set properly, so that all dependencies can be found by CMake.

```shell
git clone https://github.com/key4hep/k4LCIOReader.git
cd k4LCIOReader; mkdir build; cd build
cmake ..
make
```

## Contributing

Contributions and bug reports are welcome!
