# sensor-display
Configuración pantallas de presentación de datos generales y menú para el ajuste del reloj, temperatura, encendido de vidriera, etc.
Se utiliza un microprocesador PIC18F26J50

Las funciones que deberá representar el sistema serán las siguientes:
Presentar en el display fecha y hora.
Presentar horario de encendido y apagado de luces de la vidriera de exposición.
Indicar la temperatura del recinto, mediante la lectura de un sensor ad-hoc.
Mostrar la temperatura programada para el sistema de aire acondicionado.
Mostrar la temporada en curso (invierno / verano). 

Las acciones funcionales serán las siguientes:
Leer los datos del reloj del microcontrolador (RTCC) y representarlos en el display.
Encender y apagar las luces de vidriera en los horarios establecidos.
Medir la temperatura del recinto mediante un sensor analógico e indicarla en el display.
Activar el aire acondicionado (frio) en temporada de verano cuando la temperatura supere en 2 grados al valor de la temperatura programada y desactivarla al llegar a tal valor.
Activar la calefacción en temporada de invierno cuando la temperatura esté por debajo de los 3 grados del valor de la temperatura programada y desactivarla al llegar a tal valor.
Cambiar la temporada los días 21 de setiembre y 21 de marzo a las 0hs, según corresponda.
Un sistema de alarma se activará, dentro del horario en que la vidriera esté encendida, que sonará en caso de que los sensores de movimiento se disparen.

El menú de programaciones deberá contemplar las siguientes pautas:
Poner en fecha y hora el reloj del microcontrolador (RTCC).
Establecer los horarios de encendido y apagado de la vidriera.
Programar la temperatura de referencia para el sistema de acondicionamiento del aire.

