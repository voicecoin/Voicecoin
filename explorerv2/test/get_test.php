<?php
		$request = array(
						'method' => "getrawmempool",
						'id' => 2
						);
		$request = json_encode($request);
		$opts = array ('http' => array (
							'method'  => 'POST',
							'header'  => 'Content-type: application/json',
							'content' => $request
							));
		$context  = stream_context_create($opts);
		if ($response = file_get_contents('http://zhao:gang@127.0.0.1:6662/', false, $context)) {
			//$response = json_decode($response,true);
			echo $response;
		} else {
			throw new Exception('Unable to connect to ');
		}
		

?>
