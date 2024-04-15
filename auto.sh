sshpass -p 'nicochenlo' ssh cursoa09@ft3.cesga.es 'rm -r ArqCompP2' # Borramos la carpeta
sshpass -p 'nicochenlo' scp -r ../ArqCompP2 cursoa09@ft3.cesga.es:/mnt/netapp2/Home_FT2/home/usc/cursos/cursoa09 # Mandamos la nueva carpeta
sshpass -p 'nicochenlo' ssh cursoa09@ft3.cesga.es 'cd ArqCompP2; chmod +x scriptP2.sh; compute -c 1 --mem 2G'

