# PHP Extension for TON SDK Wrapper - Installation notes

## Requirements

 PHP 7.4+
 Windows, Linux or macOS.
 
## Windows

You can install this extension on Windows using one of the following ways.

### Option 1: PHP installation script

Run this script using the PHP interpreter you wish to install this extension to:

```
php -r "file_put_contents('windows-installer.php', fopen('https://raw.githubusercontent.com/radianceteam/ton-client-php-ext/master/windows-installer.php', 'r'));"^
 && php windows-installer.php -v 1.5.1 2>install-errors.log
```

After this, when it printed `OK` you could test installation by running this script:

```
php -r "file_put_contents('test-extension.php', fopen('https://raw.githubusercontent.com/radianceteam/ton-client-php-ext/master/test-extension.php', 'r'));"^
 && php test-extension.php -v 1.5.1 2>test-errors.log
```

In case of any issues please post new GitHub Issue in this repository and attach `install-errors.log` and `test-errors.log`
files created during installation.

### Option 2: Manual steps

Download Windows binaries from [GitHub Releases](https://github.com/radianceteam/ton-client-php-ext/releases).

How to choose archive version:

 - Check your PHP version:

```
php --version
PHP 7.4.12 (cli) (built: Oct 27 2020 17:18:47) ( ZTS Visual C++ 2017 x64 )
Copyright (c) The PHP Group
Zend Engine v3.4.0, Copyright (c) Zend Technologies
    with Xdebug v2.9.8, Copyright (c) 2002-2020, by Derick Rethans
```

Here, `7.4.12` - your PHP version, `ZTS` means that Thread Safety is enabled and `x64` is a desired platform.

Choose the corresponding archive file from GitHub releases:

![image alt ><](images/win32-release-version.png)

Archives containing `nts-` suffix in their names are Non-Thread-Safe, so need to be chosen when ZTS is not enabled.

 - Unpack the archive.
 - Copy `php_ton_client.dll` from the archive to `ext` directory in the PHP installation folder.
 - Copy `pthreadVC2.dll` and `ton_client.dll` from the archive to the PHP installation folder.
 - Modify `php.ini` file located in the PHP installation folder; add new extension:
```
extension="<Full path to php_ton_client.dll>"
```
for example:
```
extension="C:\php\ext\php_ton_client.dll"
```
 - Verify that extension is enabled by inspecting output of `php --info`:
```
php --info > phpinfo.txt 
``` 
open `phpinfo.txt` in your favourite editor and find this line:
```
ton_client

ton_client support => enabled
```
 - All done.

## Linux/Mac

There's no pre-built binaries for these platform. You will need to install 
extension by building it from sources, as described in [Development notes](development.md#linuxmac).

## Troubleshooting

Fire any question to our [Telegram channel](https://t.me/RADIANCE_TON_SDK).
 