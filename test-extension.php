<?php

class TestOptions
{
    public string $version;
    public bool $silent;
    public ?string $out_file;

    public function validate(): ?string
    {
        if (empty($this->version)) {
            return "Version not specified.";
        }
        return null;
    }

    public function __toString(): string
    {
        return <<<EOT
Version: {$this->version}
Output file: {$this->out_file}
Silent: {$this->silent}
EOT;
    }
}

function check_installed_version(TestOptions $options)
{
    $version = phpversion('ton_client');
    if (!$version) {
        fire_error("No extension installed", $options);
    }
    inform("Installed version: ${version}", $options);
    $cmp = version_compare($options->version, $version);
    if ($cmp !== 0) {
        fire_error("Different version installed: ${version}", $options);
    }
}

function append_to_file(string $file_name, string $message)
{
    $f = fopen($file_name, 'a');
    fwrite($f, $message);
    fclose($f);
}

function fire_error($message, ?TestOptions $options = null)
{
    $line = "[ERROR] ${message}\n";
    if ($options && $options->out_file) {
        append_to_file($options->out_file, $line);
    }
    if (!$options->silent) {
        fwrite(STDERR, $line);
    }
    exit(1);
}

function usage($script_name)
{
    echo <<<EOT
Usage: php ${script_name} 
    -v|--version <VERSION>      Extension version to be checked.
    [-h|--help]                 Show this help.
    [-o|--output]               Write output to file.
    [-S|--silent]               Don't print too much.
EOT;
}

function get_options($argv): TestOptions
{
    if (!($options = getopt('hSv:o:', [
        'help',
        'silent',
        'version:',
        'output:'
    ]))) {
        usage($argv[0]);
        fire_error("Failed to parse options.");
    }

    if (isset($options['h']) || isset($options['help'])) {
        usage($argv[0]);
        exit(0);
    }

    $o = new TestOptions();
    $o->version = isset($options['v']) ? $options['v'] : $options['version'];
    $o->silent = isset($options['S']) || isset($options['silent']);
    $o->out_file = isset($options['o']) ? $options['o'] : (isset($options['output']) ? $options['output'] : null);

    $errors = $o->validate();
    if (!empty($errors)) {
        usage($argv[0]);
        fire_error($errors, $o);
    }

    return $o;
}

function inform(string $message, TestOptions $options)
{
    if ($options->out_file) {
        append_to_file($options->out_file, "${message}\n");
    }
    if (!$options->silent) {
        echo "${message}\n";
    }
}

function test(TestOptions $options)
{
    inform("Running tests...", $options);

    foreach ([
                 'ton_create_context',
                 'ton_destroy_context',
                 'ton_request_sync'
             ] as $func) {
        if (!function_exists($func)) {
            fire_error("Function ${func} doesn't exist", $options);
        }
    }

    $json = json_decode(ton_create_context("{}"), true);
    if (!$json) {
        fire_error("Failed to create TON context.", $options);
    }

    $contextId = $json['result'];
    $json = ton_request_sync($contextId, 'client.version', '');
    $response = json_decode($json, true);
    if (!$response || !isset($response['result']) || !isset($response['result']['version'])) {
        fire_error("Invalid response returned by client.version: ${json}.", $options);
    }
    $version = $response['result']['version'];
    inform("Version returned by client.version: ${version}", $options);
    $cmp = version_compare($version, $options->version);
    if ($cmp !== 0) {
        fire_error("Wrong version returned by client.version: ${version}", $options);
    }
    ton_destroy_context($contextId);
}

$options = get_options($argv);

inform(<<<EOT
${options}
EOT
    , $options);

check_installed_version($options);
test($options);

inform('OK', $options);
