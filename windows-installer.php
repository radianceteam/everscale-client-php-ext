<?php

const MIN_PHP_VERSION = '7.4';

class InstallationOptions
{
    public string $version;
    public ?string $out_file;
    public string $ext_dir;
    public string $exe_dir;
    public string $tmp_dir;
    public string $ini_file;
    public string $arch;
    public bool $ts;
    public bool $force_install;
    public bool $skip_download;
    public bool $skip_cleanup;
    public bool $skip_unpack;
    public bool $skip_ini;
    public bool $skip_backup;
    public bool $silent;

    public function validate(): ?string
    {
        if (empty($this->version)) {
            return "Version not specified.";
        }
        if (!$this->skip_ini) {
            if (!is_file($this->ini_file)) {
                fire_error("PHP ini file doesn't exist: {$this->ini_file}", $this);
            }
            if (!is_writable($this->ini_file)) {
                fire_error("PHP ini file is not writable: {$this->ini_file}", $this);
            }
        }
        return null;
    }

    public function __toString(): string
    {
        return <<<EOT
Version: {$this->version}
Arch: {$this->arch}
Output file: {$this->out_file}
Extension directory: {$this->ext_dir}
PHP binary directory: {$this->exe_dir}
Temp directory: {$this->tmp_dir}
INI file: {$this->ini_file}
Thread safety: {$this->ts}
Skip download: {$this->skip_download}
Skip cleanup: {$this->skip_cleanup}
Skip unpack: {$this->skip_unpack}
Skip INI: {$this->skip_ini}
Skip backup: {$this->skip_backup}
Force install: {$this->force_install}
Silent: {$this->silent}
EOT;
    }
}

function check_installed_version(InstallationOptions $options): bool
{
    $version = phpversion('ton_client');
    if (!$version) {
        inform("No extension previously installed", $options);
        return false;
    }
    inform("Previously installed version: ${version}", $options);
    $cmp = version_compare($options->version, $version);
    if ($cmp > 0 || $options->force_install) {
        inform("Installing new version: {$options->version}", $options);
        return false;
    }
    inform("Nothing to install.", $options);
    return true;
}

function append_to_file(string $file_name, string $message)
{
    $f = fopen($file_name, 'a');
    fwrite($f, $message);
    fclose($f);
}

function fire_error(string $message, ?InstallationOptions $options = null)
{
    if ($options && $options->out_file) {
        append_to_file($options->out_file, "[ERROR] ${message}\n");
    }
    if (!$options->silent) {
        fwrite(STDERR, $message);
    }
    exit(1);
}

function usage($script_name)
{
    echo <<<EOT
Usage: php ${script_name} 
    -v|--version <VERSION>      Extension version to be installed.
    [-h|--help]                 Show this help.
    [-o|--output]               Write output to file.
    [-S|--silent]               Don't print too much.
    [-D|--skip-download]        Skip downloading archive. Use existing archive from tmp dir.
    [-U|--skip-unpack]          Skip unpacking downloaded archive.
    [-C|--skip-cleanup]         Skip removing temp files.
    [-I|--skip-ini]             Skip modifying php.ini file.
    [-B|--skip-backup]          Don't make ini file backup.
    [-f|--force-install]        Overwrite existing binaries.
    [-s|--force-thread-safe]    Force using ZTS binaries.
    [-a|--force-arch]           Force using specific architecture (x86|x64)
    [-t|--force-tmp-dir]        Force using the given tmp dir for downloading and unpacking files. 
    [-e|--force-ext-dir]        Force installing into the given EXE dir.
    [-x|--force-exe-dir]        Force installing into the given extension dir.
    [-i|--force-ini-file]       Force using the given php.ini file.
EOT;
}

function get_options($argv): InstallationOptions
{
    if (!($options = getopt('hSv:o:fDUCsa:t:e:x:i:hBI', [
        'help',
        'silent',
        'version:',
        'output:',
        'skip-download',
        'skip-unpack',
        'skip-cleanup',
        'skip-ini',
        'skip-backup',
        'force-install',
        'force-thread-safe',
        'force-arch:',
        'force-tmp-dir:',
        'force-ext-dir:',
        'force-exe-dir:',
        'force-ini-file:'
    ]))) {
        usage($argv[0]);
        fire_error("Failed to parse options.");
    }

    if (isset($options['h']) || isset($options['help'])) {
        usage($argv[0]);
        exit(0);
    }

    ob_start();
    phpinfo(INFO_GENERAL);
    $phpinfo = strip_tags(ob_get_clean());

    $o = new InstallationOptions();
    $o->version = isset($options['v']) ? $options['v'] : $options['version'];
    $o->out_file = isset($options['o']) ? $options['o'] : (isset($options['output']) ? $options['output'] : null);
    $o->skip_download = isset($options['D']) || isset($options['skip-download']);
    $o->skip_unpack = isset($options['U']) || isset($options['skip-unpack']);
    $o->skip_cleanup = isset($options['C']) || isset($options['skip-cleanup']);
    $o->skip_ini = isset($options['I']) || isset($options['skip-ini']);
    $o->skip_backup = isset($options['B']) || isset($options['skip-backup']);
    $o->force_install = isset($options['f']) || isset($options['force-install']);
    $o->silent = isset($options['S']) || isset($options['silent']);
    $o->ts = isset($options['s']) || isset($options['force-thread-safe']) || is_thread_safe($phpinfo);
    $o->arch = isset($options['a']) ? $options['a'] : (isset($options['force-arch']) ? $options['force-arch'] : get_arch($phpinfo, $o));
    $o->tmp_dir = isset($options['t']) ? $options['t'] : (isset($options['force-tmp-dir']) ? $options['force-tmp-dir'] : get_tmp_dir($o));
    $o->ext_dir = isset($options['e']) ? $options['e'] : (isset($options['force-ext-dir']) ? $options['force-ext-dir'] : get_php_ext_dir());
    $o->exe_dir = isset($options['x']) ? $options['x'] : (isset($options['force-exe-dir']) ? $options['force-exe-dir'] : get_php_exe_dir());
    $o->ini_file = isset($options['i']) ? $options['i'] : (isset($options['force-ini-file']) ? $options['force-ini-file'] : get_ini_file_location($phpinfo, $o));

    $errors = $o->validate();
    if (!empty($errors)) {
        usage($argv[0]);
        fire_error($errors, $o);
    }

    return $o;
}

function check_php_version($ver, $min, InstallationOptions $options)
{
    if (version_compare($ver, $min) >= 0) {
        inform("PHP version ${ver} >= ${min}: OK", $options);
    } else {
        fire_error("PHP version ${min}+ is required.", $options);
        exit(1);
    }
}

function check_writable($dir, InstallationOptions $options)
{
    if (is_writable($dir)) {
        inform("Directory ${dir} is writable: OK", $options);
    } else {
        fire_error("Directory ${dir} is not writable.", $options);
    }
}

function get_arch(string $phpinfo, InstallationOptions $options): string
{
    $arch = preg_match('/Architecture\s+=>\s+(\w+)/i', $phpinfo, $matches)
        ? $matches[1]
        : null;
    if (!$arch) {
        fire_error('Cannot determine OS architecture.', $options);
    }
    return $arch;
}

function is_thread_safe(string $phpinfo): bool
{
    return preg_match('/Thread\s+Safety\s+=>\s+enabled/i', $phpinfo);
}

function get_download_url(InstallationOptions $options): string
{
    $suffix = $options->ts ? '' : '-nts';
    $version = $options->version;
    $arch = $options->arch;
    return "https://github.com/radianceteam/ton-client-php-ext/releases/download/{$version}/ton-client-${version}${suffix}-Win32-vc15-${arch}.zip";
}

function download_archive(string $download_url, InstallationOptions $options): ?string
{
    $tmp_file_name = $options->tmp_dir . '\\' . basename($download_url);
    if (!$options->skip_download) {
        inform("Downloading ${download_url}...", $options);
        $f = fopen($download_url, 'r') or fire_error("Cannot download ${download_url}", $options);
        file_put_contents($tmp_file_name, $f);
        inform("Downloaded to ${tmp_file_name}", $options);
    } else {
        inform("Skipping download.", $options);
        if (!$options->skip_unpack) {
            if (!file_exists($tmp_file_name)) {
                fire_error("File not downloaded and doesn't exist: ${tmp_file_name}", $options);
            }
            inform("Using existing archive from ${tmp_file_name}.", $options);
        }
    }
    return $tmp_file_name;
}

function extract_files(ZipArchive $archive, string $dest_folder, array $files, InstallationOptions $options)
{
    $filenames = join(", ", $files);
    inform("Extracting files ${filenames} into {$options->tmp_dir}", $options);
    if (!$archive->extractTo($options->tmp_dir, $files)) {
        fire_error("Failed to extract files ${filenames} into {$options->tmp_dir}", $options);
    }
    foreach ($files as $file) {
        $basename = basename($file);
        $target_file = $dest_folder . '\\' . $basename;
        $src_file = $options->tmp_dir . '\\' . $file;
        inform("Copying ${src_file} into ${target_file}...", $options);
        if (file_exists($target_file)) {
            if (filesize($target_file) === filesize($src_file)) {
                inform("File ${target_file} already copied.", $options);
            } else {
                if (!unlink($target_file)) {
                    fire_error("Failed to remove ${target_file}.", $options);
                }
            }
        } else if (!copy($src_file, $target_file)) {
            fire_error("Failed to copy ${file} into ${target_file}.", $options);
        }
        if (!$options->skip_cleanup) {
            inform("Removing file ${src_file}.", $options);
            unlink($src_file);
        }
    }
}

function unpack_archive(string $file_name, string $folder_name, InstallationOptions $options): string
{
    if (!$options->skip_unpack) {
        inform("Unpacking and installing files...", $options);
        $zip = new ZipArchive();
        if ($zip->open($file_name) === TRUE) {
            for ($i = 0; $i < $zip->numFiles; $i++) {
                $stat = $zip->statIndex($i);
                print_r($stat['name'] . PHP_EOL);
            }
            $zip->extractTo($options->tmp_dir);
            extract_files($zip, $options->exe_dir, ["${folder_name}/pthreadVC2.dll", "${folder_name}/ton_client.dll"], $options);
            extract_files($zip, $options->ext_dir, ["${folder_name}/php_ton_client.dll"], $options);
            $zip->close();
        } else {
            fire_error("Failed to unpack ${file_name}", $options);
        }
    } else {
        inform("Skipping unpacking and installing files...", $options);
    }
    return "{$options->ext_dir}\\php_ton_client.dll";
}

function remove_archive_file(string $file_name, InstallationOptions $options)
{
    if (!$options->skip_cleanup) {
        inform("Removing archive file ${file_name}", $options);
        unlink($file_name);
    } else {
        inform("Skip removing archive file ${file_name}", $options);
    }
}

function download_and_unpack(string $download_url, InstallationOptions $options): string
{
    $zip_file_name = download_archive($download_url, $options);
    $folder_name = "build/release/{$options->arch}";
    $location = unpack_archive($zip_file_name, $folder_name, $options);
    remove_archive_file($zip_file_name, $options);
    return $location;
}

function get_php_exe_dir(): string
{
    return dirname(PHP_BINARY);
}

function get_php_ext_dir(): string
{
    return dirname(PHP_BINARY) . '\\' . ini_get('extension_dir');
}

function get_tmp_dir(InstallationOptions $options): string
{
    $sys_tmp = sys_get_temp_dir();
    $time = time();
    $dir = "${sys_tmp}\\ton_client_php_ext.${time}.tmp";
    if (!is_dir($dir) && !mkdir($dir)) {
        fire_error("Failed to create temp dir: ${dir}", $options);
    }
    return $dir;
}

function get_ini_file_location(string $phpinfo, InstallationOptions $options): string
{
    $location = preg_match('/Loaded\s+Configuration\s+File\s+=>\s+(.*?)[\s|$]/i', $phpinfo, $matches)
        ? $matches[1]
        : null;
    if (!$location) {
        fire_error('Cannot determine PHP INI file location.', $options);
    }
    return $location;
}

function check_before_install(string $php_version, InstallationOptions $options)
{
    check_writable($options->ext_dir, $options);
    check_writable($options->exe_dir, $options);
    check_php_version($php_version, MIN_PHP_VERSION, $options);
}

function backup_ini_file(InstallationOptions $options)
{
    inform("Updating {$options->ini_file}...", $options);
    if (!$options->skip_backup) {
        $backup_file = $options->ini_file . ".{$options->version}.bak";
        if (file_exists($backup_file) && filesize($backup_file) === filesize($options->ini_file)) {
            inform("Backup file ${backup_file} already exists.", $options);
        } else {
            inform("Copying {$options->ini_file} into ${backup_file}", $options);
            copy($options->ini_file, $backup_file);
        }
    } else {
        inform("Skipping making backup file for {$options->ini_file}.", $options);
    }
}

function update_extension_location(string $extension_path, InstallationOptions $options)
{
    $basename = basename($extension_path);
    $needed = "extension=\"${extension_path}\"";
    $found = false;
    $lines = [];
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
    inform("Extension location updated in {$options->ini_file}.", $options);
}

function inform(string $message, InstallationOptions $options)
{
    if ($options->out_file) {
        append_to_file($options->out_file, "${message}\n");
    }
    if (!$options->silent) {
        echo "${message}\n";
    }
}

function update_ini_file(string $extension_path, InstallationOptions $options)
{
    if (!$options->skip_ini) {
        backup_ini_file($options);
        update_extension_location($extension_path, $options);
    } else {
        inform("Skip updating init file.", $options);
    }
}

$php_version = phpversion();
$options = get_options($argv);
$download_url = get_download_url($options);

inform(<<<EOT
PHP version: {$php_version}
${options}
Download URL: ${download_url}
EOT
    , $options);

if (!check_installed_version($options)) {
    check_before_install($php_version, $options);
    $extension_path = download_and_unpack($download_url, $options);
    update_ini_file($extension_path, $options);
}

inform('OK', $options);
