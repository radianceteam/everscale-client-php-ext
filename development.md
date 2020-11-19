# PHP Extension for TON SDK Wrapper - Development notes

## Building extension

### Windows

#### Requirements

 - A 64-bit build host
 - Windows 7 or later. 
 - VS 2017. Required components:
    - C++ dev (VC15)
    - Windows SDK
    - .NET dev
 - Git

#### Running build

```
build.bat <PHP_VERSION> <PLATFORM> [<ZTS>]
```

example:

```
build.bat 7.4.1 x86
```

or

```
build.bat 7.4.1 x64 ZTS
```

### Linux/Mac

#### Required tools

`phpize` is a required command for building extension from sources.
For example, on Ubuntu it can be installed by executing `sudo apt-get install php7.4-dev` command.  

#### TON SDK installation

For installing TON client extension on Linux, first install TON client binaries:

```
./install-sdk.sh [/path/to/sdk/installation/directory]
```

Installation path can be ommitted, the script will use $HOME/ton-sdk by default.

#### Building PHP extension

 - Run build script:

```
./build.sh /path/to/sdk/installation/directory
```
which will produce lots of text but the last line will be (note 20190902 may differ):
```
Installing shared extensions:     /usr/lib/php/20190902/
```

 - Add extension to `php.ini`:

```
extension="/usr/lib/php/20190902/ton_client.so"
```

To check if the extension is loaded, call `php --info`:

```
php --info | grep ton_client
```

output should be 

```
ton_client
ton_client support => enabled
```

## Upgrading TON client library

1. Download the latest `ton_client` binaries and place to `deps` directory (replacing the existing ones).
   - binaries can be loaded from the main GitHub repository ([tonlabs/TON-SDK](https://github.com/tonlabs/TON-SDK)) or 
   from side project ([radianceteam/ton-client-dotnet-bridge](https://github.com/radianceteam/ton-client-dotnet-bridge/actions))
   which was created only fo building SDK binaries. 
   - NOTE: BE CAREFUL WITH USING BINARIES FROM TON-SDK REPO as they are only built for the major release at the moment. (Come on guys, please do a better CI/CD for this).
   - Using side project [radianceteam/ton-client-dotnet-bridge](https://github.com/radianceteam/ton-client-dotnet-bridge/actions) 
   is the recommended way ATM.
   - You could also build TON-SDK binaries yourself.
2. Re-build all binaries for Windows using `build.bat`:

```
build.bat 7.4.1 x64 ZTS
build.bat 7.4.1 x86 ZTS
build.bat 7.4.1 x64
build.bat 7.4.1 x86
```
3. Find new binaries in `build\release` directory, pack them and attach to the new release.

## Troubleshooting

Fire any question to our [Telegram channel](https://t.me/RADIANCE_TON_SDK). 

## Links

 - [TON SDK PHP client library](https://github.com/radianceteam/ton-client-php).
 - [Writing PHP Extensions](https://www.zend.com/resources/writing-php-extensions).

## TODO

1. CI/CD script for Windows binaries.
