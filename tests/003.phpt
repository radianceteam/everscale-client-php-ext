--TEST--
ton_client_test2() Basic test
--SKIPIF--
<?php
if (!extension_loaded('ton_client')) {
	echo 'skip';
}
?>
--FILE--
<?php
var_dump(ton_client_test2());
var_dump(ton_client_test2('PHP'));
?>
--EXPECT--
string(11) "Hello World"
string(9) "Hello PHP"
