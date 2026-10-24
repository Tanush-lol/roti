HEAD
wayland-project/
├── include/
│   ├── client.h
│   ├── compositor.h
│   ├── surface.h
│   ├── buffer.h
│   ├── protocol.h
│   └── types.h

g++ -Iinclude src/main.cpp -o app -lwayland-client

***practicing some wayland projects cuh i wnna build on wayland lol***

also im doing this on nixos where yea ill let u know in the first part lol
following tutorial - *https://youtu.be/v3eyPpWN4Ts* 

# Part 1
nga said we gotta get the waylandclient header file so we gotta do that except we are on nixos which is a bit harder

we need wayland-scanner which converts XML protocol file to da header 
the command is 
- *wayland-scanner client-header /usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml waylandclient.h*
this can be easily done by entering the nix-shell but we are not here to make the program reproducable rather unworkable on other machines **lol**

## error 1
[tanush@nixos:~/Projects/w11practice]$ wayland-scanner client-header /usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml waylandclient.h
bash: wayland-scanner: command not found

- alr to get started we need to find wayland-scanner on the os  
[tanush@nixos:~]$ find /nix/store -type f -name wayland-scanner
output - /nix/store/7zcpywbkgw2gx9k0sy3285awbiwg51ci-wayland-scanner-1.25.0-bin/bin/wayland-scanner

wayland-scanner = nix/store/7zcpywbkgw2gx9k0sy3285awbiwg51ci-wayland-scanner-1.25.0-bin/bin/wayland-scanner

/nix/store/7zcpywbkgw2gx9k0sy3285awbiwg51ci-wayland-scanner-1.25.0-bin/bin/wayland-scanner client-header /usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml waylandclient.h

## error 2
[tanush@nixos:~/Projects/w11practice]$ /nix/store/7zcpywbkgw2gx9k0sy3285awbiwg51ci-wayland-scanner-1.25.0-bin/bin/wayland-scanner client-header /usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml waylandclient.h
Could not open input file: No such file or directory

error directs to directory */usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml* 

[tanush@nixos:~]$ find  /nix/store -type f -name "xdg-shell.xml" 
### Output
/nix/store/bmxdr74jgv4z0bkkzjhkbmdpy9y3yvvw-qtbase-6.11.1/share/qt6/wayland/protocols/xdg-shell/xdg-shell.xml
/nix/store/hikn1nywb0xv4lshlg2v1mz1ci0z22k8-wayland-protocols-1.49/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml

we can see that the 2nd path is close to the dir we enter in the og command 

- */usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml* = 
/nix/store/hikn1nywb0xv4lshlg2v1mz1ci0z22k8-wayland-protocols-1.49/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml

## Resolution 
[tanush@nixos:~/Projects/w11practice]$ /nix/store/7zcpywbkgw2gx9k0sy3285awbiwg51ci-wayland-scanner-1.25.0-bin/bin/wayland-scanner client-header /nix/store/hikn1nywb0xv4lshlg2v1mz1ci0z22k8-wayland-protocols-1.49/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml waylandclient.h 

[tanush@nixos:~/Projects/w11practice]$ ls
readme.md  w  waylandclient.h  waytask.c

***We got the header file lol***

**also for wayland-client.h**
/nix/store/7zcpywbkgw2gx9k0sy3285awbiwg51ci-wayland-scanner-1.25.0-bin/bin/wayland-scanner client-header /nix/store/ivvsfsk2i261clp9b6l2pf006xhjp99r-wayland-scanner-1.25.0/share/wayland/wayland.xml wayland-client.h
ran this on the terminal to get the header
>>>>>>> f38928cd0c0401388e1d0af49af1a90028438527
