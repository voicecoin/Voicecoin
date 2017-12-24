<?php
error_reporting(E_ALL);
$dbconn = new mysqli("localhost", "dog","zhaogang", "emcchain");
// Check connection
if ($dbconn->connect_error) {
    die("Connection failed: " . $dbconn->connect_error);
}
else
	echo "test succa";


?>
