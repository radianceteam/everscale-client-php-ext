--TEST--
ton_client_test1() Basic test
--SKIPIF--
<?php
if (!extension_loaded('ton_client')) {
	echo 'skip';
}
?>
--FILE--
<?php
$ret = ton_client_test1();

var_dump($ret);
?>
--EXPECT--
The extension ton_client is loaded and working!
NULL
