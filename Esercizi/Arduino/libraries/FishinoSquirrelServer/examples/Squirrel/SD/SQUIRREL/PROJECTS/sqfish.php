<?php

	session_start();

	function delTree($dir)
	{
		if(empty($dir))
			return false;
		$files = array_diff(scandir($dir), array('.', '..')); 

		foreach ($files as $file) { 
			(is_dir("$dir/$file")) ? delTree("$dir/$file") : unlink("$dir/$file"); 
		}
		return rmdir($dir); 
	}
    
   	if(isset($_POST['COMMAND'])) {
		
		$cmd = $_POST['COMMAND'];
		switch($cmd) {

			case 'LOAD':
				$path = $_POST['PATH'];
				$content = file_get_contents($path);
				if($content) {
					echo '{"status":"ok"}' . "\n";
					echo $content;
				}
				else
					echo '{"status":"failed"}' . "\n";
				break;
				
			case 'STORE':
				$path = $_POST['PATH'];
				$content = $_POST['CONTENT'];
				if(!file_exists(dirname($path)))
					mkdir(dirname($path), 0777, true);
				if(file_put_contents($path, $content))
					echo '{"status":"ok"}' . "\n";
				else
					echo '{"status":"file not found"}' . "\n";
				break;
				
			case 'RMFILE':
				$path = $_POST['PATH'];
				if(is_dir($path))
					echo '{"status":"path is a directory"}' . "\n";
				else if(!file_exists($path))
					echo '{"status":"file not found"}' . "\n";
				else {
					unlink($path);
					echo '{"status":"ok"}' . "\n";
				}
				break;
				
			case 'RMFOLDER':
				$path = $_POST['PATH'];
				if(is_dir($path)) {
					if(delTree($path))
						echo '{"status":"ok"}' . "\n";
					else
						echo '{"status":"error removing folder"}' . "\n";
				}
				else
					echo '{"status":"path not found"}' . "\n";
				break;
				
			case 'LIST':
				$path = $_POST['PATH'];
				$folders = ($_POST['FOLDERS'] == "true");
				$cdir = scandir($path);
				echo '{"status":"ok"}' . "\n";
//				echo '{"folders":"' . $folders .'"}' . "\n";
				echo '{"elements":[';
				$res = "";
				foreach($cdir as $key => $value) {
					if(!in_array($value, array(".", ".."))) {
						if(is_dir($path . DIRECTORY_SEPARATOR . $value)) {
							if($folders)
								$res = $res . "\"$value\"" . ',';
						}
						else {
							if(!$folders)
								$res = $res . "\"$value\"" . ',';
						}
					}
				}
				if(substr($res, -1) == ',')
					$res = substr($res, 0, -1);
				$res = $res . ']}';
				echo $res;
				break;
				
			case 'TERMSEND':
				$content = $_POST['CONTENT'];
				$_SESSION['TERM_CMD'] = $content;
				echo '{"status":"ok"}' ."\n";
				break;
				
			case 'STATUS':
				echo '{"status":"ok"}' ."\n";
				$content = $_SESSION['TERM_CMD'];
				if($content != '') {
					echo "{\"term\": \"$content\"}";
					$_SESSION['TERM_CMD'] = '';
				}
				break;
				
			case 'RUN':
				$project = $_POST['PROJECT'];
				$path = "PROJECTS/" . $project . "/" . $project . ".nut";
				$content = file_get_contents($path);
				$_SESSION['TERM_CMD'] = $content;
				echo '{"status":"ok"}' ."\n";
				break;

			default:
				echo '{"status":"failed"}' . "\n";
		}
	}
	else
		echo '{"status":"missing command"}' . "\n";

?>
