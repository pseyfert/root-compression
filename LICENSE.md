# external inputs
 * The code which is copied over from ROOT is subject to [LGPL v2.1][lgpl2.1].
 * The LZ4 code, which is included in the repository, is subject to the BSD
   license.
 * LZO, which is *not* included in the repository is subject to [GPL v2][glp2].
   [1]
 * zopfli, which is forked for usage with this repository is licensed under the
   Apache license 2.0.
 * brotli, which is forked in this repository is licensed under the Apache
   license 2.0.
 * the interface between ROOT and LZ4 and LZO is written by Manuel Schiller.

# implications for root-compression

according to [wikipedia][foss-licenses] root-compression can only be licensed
under GPLv3, GPLv3+, or Affero GPLv3.

# icon

The Icon is derived from [iconfinder][iconfinder] published under [CC 3.0][cc3]
and the ROOT icon ([LGPL v2.1][lgpl2.1]). The resulting icon is subject to [CC
BY-SA 3.0][cc-by]. Given the incompatibility of GPL and CC, the icon must be
understood as not part of the software package root-compression.

# footnotes

[1] even w/o including LZO, linking against it still enforces GPL for
root-compression [according to
stackexchange][gpl-implications]

[foss-licenses]:      https://en.wikipedia.org/wiki/License_compatibility#Compatibility_of_FOSS_licenses
[gpl-implications]:   http://programmers.stackexchange.com/questions/158789/can-i-link-to-a-gpl-library-from-a-closed-source-application
[lgpl2.1]:            https://gnu.org/licenses/lgpl-2.1
[gpl2]:               https://gnu.org/licenses/gpl-2.0
[iconfinder]:         https://www.iconfinder.com/icons/35891/compress_icon
[cc3]:                https://creativecommons.org/licenses/by/3.0/us/
[cc-by]:              https://creativecommons.org/licenses/by-sa/3.0/
