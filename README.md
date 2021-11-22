### Dependency

No need to install liblsb.

Otherwise:

```
sudo apt-get install -y openmpi-bin libpapi-dev
sudo apt install r-base-core 
```

Then for the R dependencies run 

```
sudo Rscript ./requirement.R
```

Fedora:
```sh
sudo dnf install R-core openmpi-devel check-devel
sudo ln -s /usr/include/openmpi-x86_64/ /usr/include/openmpi
module load mpi
```