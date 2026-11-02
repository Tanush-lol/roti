# Use this command to find packages on your desktop
***find /nix/store -path "*/bin/*package_name*"**
everything on nixos is installed as /nix/store/some_random_hash_for_intallation/bin/package_name
so entering /bin/package_name would give u all enteries ending with /bin/package_name 

## find /nix/store -type d -name "*/bin/pkg-config"
specified for type search *d* for directory *f* for executables
- find /nix/store -type f -name man
similarly *s* for socket


# for nixos 
pkg-config --variable=pkgdatadir wayland-protocols

is basically for rading a package's pc file, a pc file is basically the metadata of a any package or anything u tryna get on your system right.
--variable prints all the variables in the .pc file, if u change and add a variable on your own to the .pc file,it will render that out alr, *--variable=variable_name* will return that variable *--veriable* will only return {prefix}{pkgdatadir} form the file 
- first used *find* to find directories realted to pkg-config got them 
- then we used find to find the wayland-protocols.pc 
- then used pkg-config to read out the variables from wayland-protocols.pc
  PKG_CONFIG_PATH=/nix/store/hikn1nywb0xv4lshlg2v1mz1ci0z22k8-wayland-protocols-1.49/share/pkgconfig/ 
  ***i/e declaring a path variable to let pkg-config know that we gotta use this*** 
  /nix/store/yqmfmarywhqadkkvd5w9zbz8lw9pzkyj-pkg-config-0.29.2/bin/pkg-config 
  ***the pkg-config directory***
  --variable=pkgdatadir wayland-protocols
  ***variables we wnna search for***
  //nix/store/hikn1nywb0xv4lshlg2v1mz1ci0z22k8-wayland-protocols-1.49/share/wayland-protocols
  ***this is the result***




