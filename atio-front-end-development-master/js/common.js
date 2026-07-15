const baseURL ="http://127.0.0.1:2010";
$(document).ajaxSend(function (e, xhr, opt) {
    opt.url =baseURL + opt.url;
    var token = localStorage.getItem("token");
    console.log("authorization="+token);
    xhr.setRequestHeader("authorization", token);
});

$(document).ajaxError(function(event,xhr,options,exc){
    console.log(xhr.status);
    if(xhr.status==401){
        location.href = "sign-in.html";
    }
    
});
