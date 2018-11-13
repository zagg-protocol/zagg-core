Please READ INSTALL.md

if you are facing issues with the build w.r.t postgres module rerun the config disabling the postgres module
./configure --disable-postgres

use make -j [N] to compile the source code. [N] is number of cores of your cpu; example make -j 4 if you have 4 core processor 