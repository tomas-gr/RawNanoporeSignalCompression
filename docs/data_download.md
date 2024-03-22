Para descargar datos automáticamente se provee un script de python "get_data.py"
El script se encuentra en $PROJECT_ROOT/src/python/pgnano/data_obtention/get_data.py

Antes de proceder se debe configurar adecuadamente el entorno.
Esto implica:
    - Configurar el proyecto (correr build.sh si se acaba de clonar el repositorio)
    - Activar el entorno virtual de python en /pod5/python/pod5/venv (ver: https://docs.python.org/3/library/venv.html ya que los detalles dependen del sistema operativo concreto)
    - Configurar las constantes project_root y data_root_path en /src/python/pgnano/constants/constants.py
        - project_root debe apuntar al directorio raíz del proyecto
        - data_root_path debe apuntar al directorio donde se quieren descargar los datos
    
Una vez configurado el entorno, en la raíz de datos de prueba (data_root_path de la parte anterior) se debe generar (o modificar si ya existe), el archivo data_descriptor.json con exactamente ese nombre.

El json consiste de un array de datasets. Un ejemplo minimal es el siguiente:
[
    {
        "name": "Genome in a Bottle - Ashkenazi Trio",
        "data": {
            "location": "s3",
            "post_download_script": {
                "type": "custom",
                "script": ["fast5_to_pod5", "clean_fast5"]
            },
            "link": "s3://ont-open-data/giab_lsk114_2022.12/",
            "root_link": "s3://ont-open-data/"
        },
        "metadata": {
            "web_recovered_pore_type": "R10.4.1",
            "actual_pore_type": null,
            "is_DNA": true,
            "organisms": "Human",
            "author": "ONT"
        }
    }
]

Un dataset tiene la siguiente estructura:
{
    "name": string NOT NULL,
    "data": {...} NOT NULL,
    "metadata": {...} NOT NULL
}

metadata tiene la siguiente estructura:
{
    "web_recovered_pore_type": string,
    "actual_pore_type": string,
    "is_DNA": bool NOT NULL,
    "organisms": string NOT NULL,
    "author": string NOT NULL
}

    Donde:
        - web_recovered_pore_type: Consiste en el tipo de poro que se declara por ejemplo en la página de donde se extrae el dataset.
        - actual_pore_type: El tipo de poro contenido en la metadata del archivo. En principio su valor será null a menos que se conozca el dataset por haber trabajado antes con él.
        - is_DNA: La secuenciación genómica corresponde a secuencia de ADN o ARN. Esta flag se utiliza para clasificar los datasets.
        - organisms: Organismo del cual provienen las muestras. Antes de agregar un organismo, revisar que dicha definición exista en organisms_map.json ubicado en la misma carpeta que data_descriptor.json. En dicho archivo, se le asigna a cada organismo mencionado en un dataset en data_desriptor.json, el nombre de las carpetas con el que se lo quiere representar dentro de las sistema de archivos. Así el mapping "SARS-CoV-2": "COVID" representa que un dataset con el organismo "SARS-CoV-2" se guardará en una carpeta que como parte de su nombre tendrá el string "COVID".
        - author: Autor de dataset (quien realizó el experimento). Cumple un rol análogo al campo organisms y deben agregarse asociaciones en author_map.json. 

data tiene la siguiente estructura:

{
    "location": "s3" | "script",
    "post_download_script",
    (...) // Datos dependientes de la localización de descarga
}

    Donde:
        - "location": Indica el tipo de sitio desde donde se realiza la descarga. Los tipos son:
            - "s3": Un bucket s3 de Amazon
            - "script": Un script inline de exactamente 1 linea que se ejecuta para obtener la descarga de los datos. Exactamente la linea de texto escrita aquí se pasará a la shell del sistema.
        - "post_download_script": Indica una serie de post procesados a realizar luego de la descarga. El campo tiene la estructura explicada posteriormente.

post_download_script tiene la siguiente estructura:
{
    "type": "custom" | "shell",
    "script": ordered_list(string)
}
    Donde: 
        - "type" indica el tipo de script a ejecutar.
            - "shell": Indica que se ejecute un script inline de exactamente una linea, de la shell del sistema. Actualmente no está soportado.
            - "custom": Utiliza una lista ordenada de strings particulares que implementan acciones específicas sobre los datos.
                - "fast5_to_pod5": Convierte todos los archivos descargados del dataset desde fast5 a pod5
                - "clean_fast5": Elimina archivos *.fast5 en la carpeta del dataset
                - "dummy_copy_test": Para cada pod5 del dataset intenta copiarlo trivialmente con la API de Python y verificar la equivalencia observacional del archivo con su copia. Es util cuando las descargas no proveen un checksum apropiado y para verificar que no existieron errores en la conversión de archivos.

El resto de la estructura de data depende del campo "location" indicado:
    - "script": Se agrega el campo "script": string_script_a_ejecutar
    - "s3": Se agregan los campos "link" y "root_link":
        - "link": Indica el enlace raíz s3 donde se ubican los datos. El directorio s3 será recorrido recursivamente y se hará un sampling aleatoreo de todos los fast5 encontrados en dicha recorrida
        - "root_link": Prefijo del string "link" que se utiliza para la generación de los nombres de archivos que se van a descargar. Por ejemplo, dado "link": "s3://ont-open-data/giab_lsk114_2022.12/" y "root_link": "s3://ont-open-data/" se indica al script que los archivos son nombrados por la CLI de Amazon removiendo el prefijo "s3://ont-open-data/" de "s3://ont-open-data/giab_lsk114_2022.12/" y seguido de la ruta del archivo. Correr aws s3 ls --no-sign-request --human --recursive s3://ont-open-data/giab_lsk114_2022.12/ ejemplifica como debe ser asignado el campo "root_link"

El numero de archivos descargados en el sampling y la seed utilizada debe configurarse en $PROJECT_ROOT/src/python/pgnano/constants/constants.py