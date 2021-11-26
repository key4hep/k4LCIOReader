# v00.04.00

* 2021-11-12 Placido Fernandez Declara ([PR#13](https://github.com/key4hep/k4LCIOReader/pull/13))
  - Remove `nullptr` return on `cnvAssociationCollection()` when there are no elements.
  - If no collections are found, it will return an empty `edm4hep::<something>AssociationCollection`.

* 2021-09-09 Placido Fernandez Declara ([PR#12](https://github.com/key4hep/k4LCIOReader/pull/12))
  - Adds support for TrackerHitPlane and extra associations
  - Needs EDM4hep: https://github.com/key4hep/EDM4hep/pull/122

# v00.03.02

* 2021-04-09 Placido Fernandez Declara ([PR#11](https://github.com/key4hep/k4LCIOReader/pull/11))
  - Update file handlling test

# v00.03.02

* 2021-04-09 Jiaheng Zou
  - Fixed ([issue#10](https://github.com/key4hep/k4LCIOReader/issues/10)), enable to link to objects in the same collection

# v00.03.01

* 2021-02-16 Placido Fernandez Declara ([PR#9](https://github.com/key4hep/k4LCIOReader/pull/9))
  - Exposed the LCIOConverter header to be able to use it from a different project
  - Fixed include folder not captured correctly in CMake

