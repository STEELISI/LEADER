<form action="index2.php" method="post">
	<p>email: <input type="text" name="email" /></p>
	<p><input type="submit" value="submit" /></p>
</form>
<?php
set_time_limit(0);
ini_set('memory_limit', '16384M');
ini_set("pcre.backtrack_limit", "1000000000");

	if(isset($_POST['email'])) {
		$email = $_POST['email'];
		$matching_time_start = microtime(true);
		/*for ($i = 1; $i <= 38; $i++) {
			$r = preg_match('/^([0-9a-zA-Z]([-.\w]*[0-9a-zA-Z])*@([0-9a-zA-Z][-\w]*[0-9a-zA-Z]\.)+[a-zA-Z]{2,9})$/', $email, $matches);
		}*/
		$r = preg_match('/^([0-9a-zA-Z]([-.\w]*[0-9a-zA-Z])*@([0-9a-zA-Z][-\w]*[0-9a-zA-Z]\.)+[a-zA-Z]{2,9})$/', $email, $matches);

                //$r = preg_match('^(a+)+$', $email, $matches);

		
		// ^^ This is an email validation regex from: http://regexlib.com/REDetails.aspx?regexp_id=541
		// Example of matching regexs: a@aa.aa, someone@example.com
		// The regex is vulnerable to exploit strings of the form: aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa@aa.aa@a
		$matching_time_end = microtime(true);
		$total_matching_time = ($matching_time_end - $matching_time_start);
		?> Total matching time: <?php echo $total_matching_time; ?> seconds <br> <?php
		if ($r === 1) {
			?> 
			The email address <?php echo htmlspecialchars($email); ?> is valid.<br>
			<?php
		} else if ($r === 0) {
			?>
			The email address <?php echo htmlspecialchars($email); ?> is not valid.<br>
			<?php
		} else {
			?> An error occurred
			<?php
			$err = preg_last_error();
			if ($err === PREG_NO_ERROR) {
				echo "An unknown error occurred<br>\n" ;
			}else if ($err === PREG_BACKTRACK_LIMIT_ERROR) {
				echo "Backtrack limit reached before match completed<br>\n";
			} else {
				echo "Some other error occurred, code: " . $err . "<br>\n";
			}
		}
	}

?>

