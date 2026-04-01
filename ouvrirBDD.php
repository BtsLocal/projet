<?php
try{
	$user="root";
	$password="";
	$base="temperature_local_server";
	$host="localhost";
	$SGBD="mysql:host=$host;dbname=$base";
	$options[PDO::ATTR_ERRMODE]=PDO::ERRMODE_EXCEPTION;
	
	$bdd = new PDO($SGBD, $user, $password, $options);
	echo "Ouverte de la base: OK";
}
catch(Exception $e){
    die('Erreur : ' . $e->getMessage());
}
?>
