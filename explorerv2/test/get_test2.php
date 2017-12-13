<?php
	$request = array(
						'method' => "getrawmempool",
						'id' => 22
						);
		$request = json_encode($request);
		$opts = array ('http' => array (
							'method'  => 'POST',
							'header'  => 'Content-type: application/json',
							'content' => $request
							));
		$context  = stream_context_create($opts);
		if ($fp = fopen('http://zhao:gang@192.168.111.132:6662/', 'r', false, $context)) {
			$response = '';
			while($row = fgets($fp)) {
				$response.= trim($row)."\n";
			}
			//$response = json_decode($response,true);
			echo $response;
		} else {
			throw new Exception('Unable to connect to ');
		}
?>
