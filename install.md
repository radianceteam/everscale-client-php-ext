# PHP Extension for TON SDK Wrapper - Installation notes

## Requirements

 PHP 7.4+
 Windows, Linux or macOS.
 
You can install this extension using one of the following ways.

### Option 1: PHP installation script

Run this script using the PHP interpreter you wish to add this extension to:

```
php -r "copy('https://raw.githubusercontent.com/radianceteam/ton-client-php-ext/master/installer.php', 'installer.php');"
php -r "if (hash_file('sha384', 'installer.php') === 'e4cf204ea6127408c252d2addc7b533abe603cf949be32dfd35b871fea82c814684acd382cfc0b492c613280cd7e9239') { echo 'Installer verified'; } else { echo 'Installer corrupt'; } echo PHP_EOL;"
php installer.php -v 1.5.1
php installer.php -v 1.5.1 -T
```

On Linux you may need to run installation step using `sudo`:

```
sudo php installer.php -v 1.5.1
```

The last command `php installer.php -v 1.5.1 -T` is to test the new installation.

In case of any issues please re-run the problematic command with additional arguments `-o install.log -V`
and post new GitHub Issue in this repository with `install.log` file created during installation.

For example:

```
php installer.php -v 1.5.1 -o install.log -V
```

### Option 2: Manual steps

#### Windows

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

#### Linux/Mac

There's no pre-built binaries for these platform. You will need to install
extension by building it from sources, as described in [Development notes](development.md#linuxmac).

## Troubleshooting

Fire any question to our [Telegram channel](https://t.me/RADIANCE_TON_SDK).
 