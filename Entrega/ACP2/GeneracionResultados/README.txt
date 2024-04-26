En esta carpeta se incluyen un par de script, uno de bash y uno de R,
con los que se pueden replicar los resultados y las gráficas que se muestran
en el informe.

El script "ejecucion.sh" debe ejecutarse en la misma carpeta que los archivos
de código fuente de los programas, lo que producirá por terminal una salida
con los resultados en formato CSV. Para guardar el resultado, redirigir
la salida a un archivo llamado "out" (es lo que espera el script de R).

Una vez de disponga de ese archivo "out" con los datos, si se coloca en la misma
carpeta que el script de R "generate_plots.r" y se ejecuta este: `Rscript generate_plots.r`,
se generarán las gráficas que se incluyen en el informe.

Siéntase libre de modificar cualquiera de estos dos scripts para obtener
los resultados que se deseen.
