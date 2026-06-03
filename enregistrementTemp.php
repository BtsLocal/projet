<?php

    $temp = $_GET['temp'];

    $compteur = $_GET['compteur'];
    $date = date('Y-m-d');
    $heure = date('H:i:s');


    include ('ouvrirBDD.php');
  

    try
    {

    
       $sql = "INSERT INTO donnees_local (temperature, compteur_personne, horaire, date) VALUES ('$temp', '$compteur', '$heure', '$date')";
       $bdd->exec($sql);
       echo "Enregistrement fait";
    }   
    catch (Exception $e)
    {
        die('Erreur : ' . $e->getMessage());
        echo "Erreur";
    }

    $bdd = null;
?> 
