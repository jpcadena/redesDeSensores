"""
Redes de Sensores
"""

#importacion de librerias
import pandas as pd #dataframe
import seaborn as sns #heatmap
import matplotlib.pyplot as plt #graficas
import pymysql #comunicacion con la DB

#listas vacias
temperature = []
power = []

#se establece conexion con los datos proporcionados de la DB(proyecto), usuario y contrasena
mySQLconnection = pymysql.connect(host='localhost',
                         database='proyecto',
                         user='jpcadena',
                         password='root1234')

cursor = mySQLconnection.cursor() #se genera un cursor
#se realiza una consulta de los datos de temperatura de la tabla sensores
sql_query = "SELECT * FROM sensores"
cursor.execute(sql_query)
rcount = int(cursor.rowcount)
for r in rcount: #se recorre el cursor
	row = cursor.fetchone() #se agregan los elementos a las listas respectivas
    temperature.append(row[1])
    power.append(row[2])
cursor.close() #cerrando las conexiones
mySQLconnection.close()

print(temperature)
print(power)

#se genera el DF
data = pd.DataFrame(data={'temperatura':temperature, 'potencia':power})
data = data.pivot(index='Yrows', columns='Xcols', values='Potencia')

#definiendo la grafica
fig, ax = plt.subplots(figsize=(12,7))

#titulo
title = "Mapa de calor de focos xd"

#tamaño y diseño
plt.title(title, fontsize=18)
ttl = ax.title
ttl.set_position([0.5, 1.05])

#ocultando ticks para los ejes
ax.set_xticks([])
ax.set_yticks([])

#eliminando axes
ax.axis('off')

#generando el mapa
sns.heatmap(data, fmt="", cmap='RdYlGn', linewidths=0.3, ax=ax)

#mostrando
plt.show()