// Checksum the names and values of all datafields, return the checksum
//   form:  The form to authenticate
//   "cookie"  The field value to contain the checksum (required)
//   "salt"    The field containing the crypt salt (optional)

function authenticate(form) {
    result = new String()
    for (i=0;i<form.elements.length;i++) {
	if (form.elements[i].name != "cookie") {
	    result += form.elements[i].name
	    result += form.elements[i].value
	}
    }
    form.cookie.value = document.myApp.digest(result)
    return true
}
