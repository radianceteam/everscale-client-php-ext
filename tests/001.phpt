--TEST--
Check if ton_client is loaded
--SKIPIF--
<?php
if (!extension_loaded('ton_client')) {
	echo 'skip';
}
?>
--FILE--
<?php
echo 'The extension "ton_client" is available';
?>
--EXPECT--
The extension "ton_client" is available
