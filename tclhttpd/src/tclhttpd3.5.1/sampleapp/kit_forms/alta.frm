[Author "Rafael Gonzalez Fuentetaja"]
[Meta generator {tclhttpd templates}]
[Header "Alta de usuario"]

[DBSetTag     corweb]
[DBSetView    v_users]
[DBSetKeyName login]

<H2> Paso 1 de 3 </H2>
Introducir el identificador de usuario nuevo (máx 16 caracteres). 
Cada usuario deberá tener un identificador único. Este identificador se empleará en la autenticación.
<DIV align=center>
[InputForm alta1form {
     1 text		login		"Identificador sugerido"
	} /adminweb/usuarios/alta2.frm /adminweb/usuarios/index.htm NewUserFormHandler 16]
</DIV>
[Footer]
