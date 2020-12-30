<?php declare(strict_types=1);

const MIN_PHP_VERSION = '7.4';

class InstallationOptions
{
    public string $version;
    public ?string $out_file;
    public string $ext_dir;
    public string $exe_dir;
    public string $tmp_dir;
    public bool $tmp_dir_created = false;
    public string $sdk_dir;
    public string $ini_file;
    public string $ini_dir;
    public string $arch;
    public bool $windows;
    public bool $ts;
    public bool $force_install;
    public bool $skip_download;
    public bool $skip_cleanup;
    public bool $skip_unpack;
    public bool $skip_ini;
    public bool $skip_backup;
    public bool $skip_build;
    public bool $silent;
    public bool $verbose;
    public bool $test;

    private function __construct()
    {
    }

    public static function usage($script_name)
    {
        print_message(<<<EOT
Usage: php ${script_name} 
    -v|--version <VERSION>      Extension version to be installed.
    [-h|--help]                 Show this help.
    [-o|--output]               Write output to file.
    [-S|--silent]               Don't print too much.
    [-V|--verbose]              Print extra information.
    [-D|--skip-download]        Skip downloading archive. Use existing archive from tmp dir.
    [-U|--skip-unpack]          Skip unpacking downloaded archive.
    [-C|--skip-cleanup]         Skip removing temp files.
    [-I|--skip-ini]             Skip modifying php.ini file (Windows only).
    [-B|--skip-backup]          Don't make ini file backup (Windows only).
    [-X|--skip-build]           Don't build sources (Linux only).
    [-T|--test-only]            Test installation.
    [-f|--force-install]        Overwrite existing binaries.
    [-s|--force-thread-safe]    Force using ZTS binaries.
    [-a|--force-arch]           Force using specific architecture (x86|x64) (Windows only).
    [-t|--force-tmp-dir]        Force using the given tmp dir for downloading and unpacking files. 
    [-e|--force-ext-dir]        Force installing into the given EXE dir.
    [-x|--force-exe-dir]        Force installing into the given extension dir.
    [-i|--force-ini-file]       Force using the given php.ini file (Windows only).
    [-d|--force-sdk-dir]        TON SDK installation directory (Linux only). Default: php extension directory.
EOT
        );
    }

    public function validate(): ?string
    {
        if (empty($this->version)) {
            return "Version not specified.";
        }
        if ($this->windows) {
            if (!$this->skip_ini) {
                if (!$this->ini_file) {
                    return 'Cannot determine PHP INI file location.';
                }
                if (!is_file($this->ini_file)) {
                    return "PHP ini file doesn't exist: {$this->ini_file}";
                }
                if (!is_writable($this->ini_file)) {
                    return "PHP ini file is not writable: {$this->ini_file}";
                }
            }
            if (!$this->arch) {
                return 'Cannot determine OS architecture.';
            }
        } else {
            if (!$this->ini_dir) {
                return 'Cannot determine PHP INI files location.';
            }
            if (!is_dir($this->ini_dir)) {
                return "{$this->ini_dir} is not a directory.";
            }
            if (!is_writable($this->ini_dir)) {
                return "Directory {$this->ini_dir} is not writable.";
            }
        }
        if (!is_dir($this->tmp_dir)) {
            if (mkdir($this->tmp_dir)) {
                $this->tmp_dir_created = true;
            } else {
                return "Failed to create temp dir: {$this->tmp_dir}";
            }
        }
        return null;
    }

    private function bool_str(bool $b): string
    {
        return $b ? 'true' : 'false';
    }

    public function __toString(): string
    {
        return <<<EOT
Version: {$this->version}
Windows OS: {$this->windows}
Arch: {$this->arch}
Output file: {$this->out_file}
Extension directory: {$this->ext_dir}
PHP binary directory: {$this->exe_dir}
SDK directory: {$this->sdk_dir}
Temp directory: {$this->tmp_dir}
INI file: {$this->ini_file}
INI dir: {$this->ini_dir}
Thread safety: {$this->bool_str($this->ts)}
Skip download: {$this->bool_str($this->skip_download)}
Skip cleanup: {$this->bool_str($this->skip_cleanup)}
Skip unpack: {$this->bool_str($this->skip_unpack)}
Skip INI: {$this->bool_str($this->skip_ini)}
Skip backup: {$this->bool_str($this->skip_backup)}
Skip build: {$this->bool_str($this->skip_build)}
Force install: {$this->bool_str($this->force_install)}
Silent: {$this->bool_str($this->silent)}
Verbose: {$this->bool_str($this->verbose)}
Test: {$this->bool_str($this->test)}
EOT;
    }

    public static function parse(array $argv): InstallationOptions
    {
        if (!($options = getopt('hSVv:o:fDUCsa:t:e:x:i:d:hBITX', [
            'help',
            'silent',
            'verbose',
            'version:',
            'output:',
            'skip-download',
            'skip-unpack',
            'skip-cleanup',
            'skip-ini',
            'skip-backup',
            'skip-build',
            'test-only',
            'force-install',
            'force-thread-safe',
            'force-arch:',
            'force-tmp-dir:',
            'force-ext-dir:',
            'force-exe-dir:',
            'force-ini-file:',
            'force-sdk-dir:'
        ]))) {
            self::usage($argv[0]);
            fire_error("Failed to parse options.");
        }

        if (isset($options['h']) || isset($options['help'])) {
            self::usage($argv[0]);
            exit(0);
        }

        ob_start();
        phpinfo(INFO_GENERAL);
        $phpinfo = strip_tags(ob_get_clean());

        $o = new InstallationOptions();
        $o->windows = is_windows($phpinfo);
        $o->version = isset($options['v']) ? $options['v'] : $options['version'];
        $o->out_file = isset($options['o']) ? $options['o'] : (isset($options['output']) ? $options['output'] : null);
        $o->skip_download = isset($options['D']) || isset($options['skip-download']);
        $o->skip_unpack = isset($options['U']) || isset($options['skip-unpack']);
        $o->skip_cleanup = isset($options['C']) || isset($options['skip-cleanup']);
        $o->skip_ini = isset($options['I']) || isset($options['skip-ini']);
        $o->skip_backup = isset($options['B']) || isset($options['skip-backup']);
        $o->skip_build = isset($options['X']) || isset($options['skip-build']);
        $o->force_install = isset($options['f']) || isset($options['force-install']);
        $o->silent = isset($options['S']) || isset($options['silent']);
        $o->verbose = isset($options['V']) || isset($options['verbose']);
        $o->test = isset($options['T']) || isset($options['test-only']);
        $o->ts = isset($options['s']) || isset($options['force-thread-safe']) || is_thread_safe($phpinfo);
        $o->arch = isset($options['a']) ? $options['a'] : (isset($options['force-arch']) ? $options['force-arch'] : get_arch($phpinfo));
        $o->tmp_dir = isset($options['t']) ? $options['t'] : (isset($options['force-tmp-dir']) ? $options['force-tmp-dir'] : get_tmp_dir());
        $o->ext_dir = isset($options['e']) ? $options['e'] : (isset($options['force-ext-dir']) ? $options['force-ext-dir'] : get_php_ext_dir());
        $o->exe_dir = isset($options['x']) ? $options['x'] : (isset($options['force-exe-dir']) ? $options['force-exe-dir'] : get_php_exe_dir());
        $o->ini_file = isset($options['i']) ? $options['i'] : (isset($options['force-ini-file']) ? $options['force-ini-file'] : get_ini_file_location($phpinfo));
        $o->sdk_dir = isset($options['d']) ? $options['d'] : (isset($options['sdk-dir']) ? $options['force-sdk-dir'] : $o->ext_dir . DIRECTORY_SEPARATOR . 'ton-sdk');
        $o->ini_dir = get_ini_dir_location($phpinfo);

        $errors = $o->validate();
        if (!empty($errors)) {
            self::usage($argv[0]);
            fire_error($errors);
        }

        return $o;
    }
}

function print_message(string $message)
{
    echo "${message}\n";
}

function fire_error(string $message)
{
    fwrite(STDERR, "${message}\n");
    exit(1);
}

function append_to_file(string $file_name, string $message)
{
    $f = fopen($file_name, 'a');
    fwrite($f, "${message}\n");
    fclose($f);
}

function is_windows(string $phpinfo): bool
{
    $system = preg_match('/System\s+=>\s+(\w+)/i', $phpinfo, $matches)
        ? $matches[1]
        : null;
    return strpos(strtolower($system), 'windows') !== false;
}

function get_arch(string $phpinfo): string
{
    $arch = preg_match('/Architecture\s+=>\s+(\w+)/i', $phpinfo, $matches)
        ? $matches[1]
        : null;
    if (!$arch) {
        return '';
    }
    return $arch;
}

function is_thread_safe(string $phpinfo): bool
{
    return !!preg_match('/Thread\s+Safety\s+=>\s+enabled/i', $phpinfo);
}

function get_ini_file_location(string $phpinfo): string
{
    return preg_match('/Loaded\s+Configuration\s+File\s+=>\s+(.*?)[\s|$]/i', $phpinfo, $matches)
        ? $matches[1]
        : '';
}

function get_ini_dir_location(string $phpinfo): string
{
    return preg_match('/Scan this dir for additional \.ini files\s+=>\s+(.*?)[\s|$]/i', $phpinfo, $matches)
        ? $matches[1]
        : '';
}

function get_php_exe_dir(): string
{
    return dirname(PHP_BINARY);
}

function get_php_ext_dir(): string
{
    $ext_dir = ini_get('extension_dir');
    if (is_dir($ext_dir)) {
        return $ext_dir;
    }
    if (is_dir(dirname(PHP_BINARY) . DIRECTORY_SEPARATOR . $ext_dir)) {
        return dirname(PHP_BINARY) . DIRECTORY_SEPARATOR . $ext_dir;
    }
    return '';
}

function get_tmp_dir(): string
{
    $sys_tmp = sys_get_temp_dir();
    $time = time();
    return $sys_tmp . DIRECTORY_SEPARATOR . "ton_client_php_ext.${time}.tmp";
}

interface InstallerInterface
{
    /**
     * Install TON Client extension.
     */
    function install(): void;

    /**
     * Test installation.
     * Should not be called right after install.
     * Call it in a separate script instead.
     * Otherwise it may not find extension.
     */
    function test(): void;
}

abstract class AbstractInstaller implements InstallerInterface
{
    protected InstallationOptions $_options;

    /**
     * AbstractInstaller constructor.
     * @param InstallationOptions $_options
     */
    public function __construct(InstallationOptions $_options)
    {
        $this->_options = $_options;
    }

    public final function install(): void
    {
        $options = $this->_options;

        try {
            $php_version = phpversion();
            $download_url = $this->getDownloadUrl();

            $this->verbose(<<<EOT
TON SDK {$options->version} PHP EXTENSION INSTALLER 
PHP version: {$php_version}
Download URL: ${download_url}
{$options}
EOT
            );

            if (!$this->checkInstalledVersion()) {
                $this->checkBeforeInstall($php_version);
                $this->downloadAndUnpack($download_url);
            }

            $this->inform('OK');

        } catch (RuntimeException $e) {
            $message = $e->getMessage();
            if ($options->out_file) {
                append_to_file($options->out_file, "[ERROR] {$message}");
            }
            if (!$options->silent) {
                fwrite(STDERR, "${message}\n");
            }
        } finally {
            if ($options->tmp_dir_created && !$options->skip_cleanup) {
                $this->inform("Removing tmp dir {$options->tmp_dir}.");
                if (!$this->deleteDirectory($options->tmp_dir)) {
                    $this->warn("Failed to remove tmp dir {$options->tmp_dir}.");
                }
            }
        }
    }

    public final function test(): void
    {
        $options = $this->_options;
        $this->verbose("Running tests...");

        foreach ([
                     'ton_create_context',
                     'ton_destroy_context',
                     'ton_request_sync'
                 ] as $func) {
            if (!function_exists($func)) {
                $this->error("Function ${func} doesn't exist");
            }
        }

        $json = json_decode(ton_create_context("{}"), true);
        if (!$json) {
            $this->error("Failed to create TON context.");
        }

        $contextId = $json['result'];
        $json = ton_request_sync($contextId, 'client.version', '');
        $response = json_decode($json, true);
        if (!$response || !isset($response['result']) || !isset($response['result']['version'])) {
            $this->error("Invalid response returned by client.version: ${json}.");
        }
        $version = $response['result']['version'];
        $this->inform("Version returned by client.version: ${version}");
        $cmp = version_compare($version, $options->version);
        if ($cmp !== 0) {
            $this->error("Wrong version returned by client.version: ${version}");
        }
        ton_destroy_context($contextId);
        $this->inform('OK');
    }

    private function handleError(RuntimeException $e)
    {
        $options = $this->_options;
        $message = $e->getMessage();
        if ($options->out_file) {
            append_to_file($options->out_file, "[ERROR] {$message}");
        }
        if (!$options->silent) {
            fwrite(STDERR, "${message}\n");
        }
    }

    protected function inform(string $message): void
    {
        $options = $this->_options;
        if ($options->out_file) {
            append_to_file($options->out_file, $message);
        }
        if (!$options->silent) {
            print_message($message);
        }
    }

    protected function verbose(string $message): void
    {
        if ($this->_options->verbose) {
            $this->inform($message);
        }
    }

    protected function error(string $message): void
    {
        throw new RuntimeException($message);
    }

    protected function warn(string $message): void
    {
        $this->inform("WARNING: ${message}");
    }

    protected abstract function getDownloadUrl(): string;

    /**
     * @param string $file_name Path to downloaded archive.
     */
    protected abstract function unpackArchive(string $file_name): void;

    protected function downloadAndUnpack(string $download_url): void
    {
        $zip_file_name = $this->downloadArchive($download_url);
        $this->unpackArchive($zip_file_name);
        $this->removeArchiveFile($zip_file_name);
    }

    protected function downloadArchive(string $download_url): ?string
    {
        $options = $this->_options;
        $tmp_file_name = $options->tmp_dir . DIRECTORY_SEPARATOR . basename($download_url);
        if (!$options->skip_download) {
            $this->inform("Downloading ${download_url}...");
            $f = fopen($download_url, 'r') or $this->error("Cannot download ${download_url}");
            file_put_contents($tmp_file_name, $f);
            $this->inform("Downloaded to ${tmp_file_name}.");
        } else {
            $this->inform("Skipping download.");
            if (!$options->skip_unpack) {
                if (!file_exists($tmp_file_name)) {
                    $this->error("File not downloaded and doesn't exist: ${tmp_file_name}");
                }
                $this->inform("Using existing archive from ${tmp_file_name}.");
            }
        }
        return $tmp_file_name;
    }

    function removeArchiveFile(string $file_name)
    {
        if (!$this->_options->skip_cleanup) {
            $this->deleteFile($file_name);
        } else {
            $this->inform("Skip removing archive file ${file_name}");
        }
    }

    function checkPhpVersion($ver, $min)
    {
        if (version_compare($ver, $min) >= 0) {
            $this->verbose("PHP version ${ver} >= ${min}: OK");
        } else {
            fire_error("PHP version ${min}+ is required.");
            exit(1);
        }
    }

    function checkWritable($dir)
    {
        if (is_writable($dir)) {
            $this->verbose("Directory ${dir} is writable: OK");
        } else {
            fire_error("Directory ${dir} is not writable (try running by superuser account?).");
        }
    }

    protected function checkInstalledVersion(): bool
    {
        $options = $this->_options;
        $version = phpversion('ton_client');
        if (!$version) {
            $this->inform("No extension previously installed");
            return false;
        }
        $this->inform("Previously installed version: ${version}");
        $cmp = version_compare($options->version, $version);
        if ($cmp > 0 || $options->force_install) {
            $this->inform("Installing new version: {$options->version}");
            return false;
        }
        $this->inform("Nothing to install.");
        return true;
    }

    protected function checkBeforeInstall(string $php_version)
    {
        $options = $this->_options;
        $this->checkPhpVersion($php_version, MIN_PHP_VERSION);
        $this->checkWritable($options->ext_dir);
        $this->checkWritable($options->exe_dir);
    }

    protected function deleteFile(string $file_name): bool
    {
        $this->inform("Removing file ${file_name}.");
        if (!$this->_options->windows) {
            if (!unlink($file_name)) {
                return false;
            }
        } else {
            $lines = array();
            exec("DEL /F/Q \"$file_name\"", $lines, $error);
            return !$error;
        }
        return true;
    }

    protected function deleteDirectory(string $dir): bool
    {
        if (!file_exists($dir)) {
            return true;
        }
        if (!is_dir($dir)) {
            return $this->deleteFile($dir);
        }
        foreach (scandir($dir) as $item) {
            if ($item == '.' || $item == '..') {
                continue;
            }
            if (!$this->deleteDirectory($dir . DIRECTORY_SEPARATOR . $item)) {
                return false;
            }
        }
        return rmdir($dir);
    }
}

class WindowsInstaller extends AbstractInstaller
{
    protected function getDownloadUrl(): string
    {
        $options = $this->_options;
        $suffix = $options->ts ? '' : '-nts';
        $version = $options->version;
        $arch = $options->arch;
        return "https://github.com/radianceteam/ton-client-php-ext/releases/download/{$version}/ton-client-${version}${suffix}-Win32-vc15-${arch}.zip";
    }

    protected function unpackArchive(string $file_name): void
    {
        $options = $this->_options;
        $folder_name = "build/release/{$options->arch}";
        if (!$options->skip_unpack) {
            $this->inform("Unpacking and installing files...");
            $zip = new ZipArchive();
            if ($zip->open($file_name) === TRUE) {
                $this->extractFiles($zip, $options->exe_dir, ["${folder_name}/pthreadVC2.dll", "${folder_name}/ton_client.dll"]);
                $this->extractFiles($zip, $options->ext_dir, ["${folder_name}/php_ton_client.dll"]);
                $zip->close();
            } else {
                $this->error("Failed to unpack ${file_name}");
            }
        } else {
            $this->inform("Skipping unpacking and installing files...");
        }
        $this->updateIniFile($options->ext_dir . DIRECTORY_SEPARATOR . "php_ton_client.dll");
    }

    private function extractFiles(ZipArchive $archive, string $dest_folder, array $files)
    {
        $options = $this->_options;
        $filenames = join(", ", $files);
        $this->inform("Extracting files ${filenames} into {$options->tmp_dir}");
        if (!$archive->extractTo($options->tmp_dir, $files)) {
            fire_error("Failed to extract files ${filenames} into {$options->tmp_dir}");
        }
        foreach ($files as $file) {
            $basename = basename($file);
            $target_file = $dest_folder . DIRECTORY_SEPARATOR . $basename;
            $src_file = $options->tmp_dir . DIRECTORY_SEPARATOR . str_replace('/', DIRECTORY_SEPARATOR, $file);
            $this->inform("Copying ${src_file} into ${target_file}...");
            if (file_exists($target_file)) {
                if (filesize($target_file) === filesize($src_file)) {
                    $this->inform("File ${target_file} already copied.");
                } else {
                    if (!$this->deleteFile($target_file)) {
                        fire_error("Failed to remove ${target_file}.");
                    }
                }
            } else if (!copy($src_file, $target_file)) {
                fire_error("Failed to copy ${file} into ${target_file}.");
            }
            if (!$options->skip_cleanup) {
                $this->deleteFile($src_file);
            }
        }
    }

    function backupIniFile()
    {
        $options = $this->_options;
        $this->inform("Updating {$options->ini_file}...");
        if (!$options->skip_backup) {
            $backup_file = $options->ini_file . ".{$options->version}.bak";
            if (file_exists($backup_file) && filesize($backup_file) === filesize($options->ini_file)) {
                $this->inform("Backup file ${backup_file} already exists.");
            } else {
                $this->inform("Copying {$options->ini_file} into ${backup_file}");
                copy($options->ini_file, $backup_file);
            }
        } else {
            $this->inform("Skipping making backup file for {$options->ini_file}.");
        }
    }

    private function updateExtensionLocation(string $extension_path): void
    {
        $basename = basename($extension_path);
        $needed = "extension=\"${extension_path}\"";
        $found = false;
        $lines = [];
        $options = $this->_options;
        foreach (explode("\n", file_get_contents($options->ini_file)) as $line) {
            $path = preg_match("/extension=(.*?)${basename}/i", $line, $matches)
                ? $matches[1]
                : null;
            if ($path) {
                $lines[] = $needed;
                $found = true;
            } else {
                $lines[] = $line;
            }
        }
        if (!$found) {
            $lines[] = $needed;
        }
        file_put_contents($options->ini_file, join("\n", $lines));
        $this->inform("Extension location updated in {$options->ini_file}.");
    }

    private function updateIniFile(string $extension_path): void
    {
        if (!$this->_options->skip_ini) {
            $this->backupIniFile();
            $this->updateExtensionLocation($extension_path);
        } else {
            $this->inform("Skip updating init file.");
        }
    }
}

class LinuxInstaller extends AbstractInstaller
{
    protected function getDownloadUrl(): string
    {
        $version = $this->_options->version;
        return "https://github.com/radianceteam/ton-client-php-ext/archive/${version}.tar.gz";
    }

    protected function checkBeforeInstall(string $php_version)
    {
        if (linux_command_exist('phpize')) {
            $this->verbose("phpize command exists: OK");
        } else {
            $this->error('phpize command doesn\'t exist. Install php-dev package (on Linux) or php via Homebrew (on Mac)');
        }

        parent::checkBeforeInstall($php_version);
    }

    protected function unpackArchive(string $file_name): void
    {
        $options = $this->_options;
        $source_dir = $options->tmp_dir . DIRECTORY_SEPARATOR . "ton-client-php-ext-{$options->version}";
        if (is_dir($source_dir)) {
            $this->inform("Directory ${source_dir} already exists.");
        } else if (!$options->skip_unpack) {
            $this->inform("Unpacking and installing files...");
            $archive = new PharData($file_name);
            if (!$archive->extractTo($options->tmp_dir)) {
                $this->error("Failed to unpack ${file_name}");
            }
        } else {
            $this->inform("Skipping unpacking and installing files...");
        }
        if (!is_dir($source_dir)) {
            $this->error("${source_dir} is not a directory");
        }
        $this->installSdk($source_dir);
        $ext_file = $this->installExtension($source_dir);
        $this->writeIniFile($ext_file);
    }

    private function installSdk(string $dir): void
    {
        $sdk_dir = $this->_options->sdk_dir;
        if (!system("${dir}/install-sdk.sh '${sdk_dir}'")) {
            $this->error("Failed to install TON SDK into ${sdk_dir}");
        }
    }

    private function installExtension(string $dir): string
    {
        $sdk_dir = $this->_options->sdk_dir;
        $ext_dir = $this->_options->ext_dir;

        if (!$this->_options->skip_build) {
            $this->verbose("Building PHP extension...");
            system("${dir}/build.sh '${sdk_dir}'");
        } else {
            $this->inform("Skipping build.");
        }

        $ext_file = "${dir}/build/modules/ton_client.so";
        if (!file_exists($ext_file)) {
            $this->error("Cannot find TON Client PHP extension file ${ext_file}.");
        }
        $this->verbose("Copying extension file ${ext_file} into ${ext_dir}.");
        $ext_file_target = "${ext_dir}/ton_client.so";
        copy($ext_file, $ext_file_target);
        $this->inform("Extension file ${ext_file} copied into ${ext_dir}.");

        return $ext_file;
    }

    private function writeIniFile(string $ext_file): void
    {
        $ini_file = $this->_options->ini_dir . '/ton_client.ini';
        if (file_put_contents($ini_file, 'extension="ton_client.so"') === false) {
            $this->error("Cannot enable extension by modifying ${ini_file}.");
        } else {
            $this->inform("Extension is enabled in ${ini_file}.");
        }
    }
}

function linux_command_exist(string $cmd): bool
{
    return !empty(shell_exec(sprintf("which %s", escapeshellarg($cmd))));
}

$options = InstallationOptions::parse($argv);

$installer = $options->windows
    ? new WindowsInstaller($options)
    : new LinuxInstaller($options);

if ($options->test) {
    $installer->test();
} else {
    $installer->install();
}
