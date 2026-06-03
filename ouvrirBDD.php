<?php
try{
	$user="root";
	$password="";
	$base="projet";
	$host="localhost";
	$SGBD="mysql:host=$host;dbname=$base";
	$options[PDO::ATTR_ERRMODE]=PDO::ERRMODE_EXCEPTION;
	
	$bdd = new PDO($SGBD, $user, $password, $options);
	
}
catch(Exception $e){
    die('Erreur : ' . $e->getMessage());
}
?>
