//void *monitor_routine(void *ptr)
/* Une boucle while très rapide qui parcourt le tableau de codeurs.
•	Si get_time() - coder->last_compile_start > time_to_burnout, alors :
•	Verrouiller le mutex log.
•	Afficher le burnout.
•	Passer data->simulation_running à 0.
•	Sortir de la boucle. */