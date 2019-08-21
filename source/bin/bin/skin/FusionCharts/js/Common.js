
//全局变量：
var ISLOGGING = 1; //是否允许将信息写入日志 0为不写入日志 1为写入日志

//替换全部内容
function replaceAllStr(inputString, strFind, strReplace) {
    ///<summary>
    ///替换全部内容
    ///</summary>
    ///<param name="inputString" type="String">
    ///源字符串
    ///</param>
    ///<param name="strFind" type="String">
    ///匹配串
    ///</param>
    ///<param name="strReplace" type="String">
    ///目标串
    ///</param>
    ///<returns type="String" >
    ///
    ///</returns>
    var reg = new RegExp("(" + strFind + ")", "g");
    return inputString.replace(reg, strReplace);
}

//将信息写入日志
function loggingMessage(message) {
    ///<summary>
    ///将信息写入日志
    ///</summary>
    ///<param name="message" type="String">
    ///日志内容
    ///</param>
    ///<returns type="void" >
    ///
    ///</returns>
    if (!ISLOGGING) return;
    $.ajax({
        type: "POST", async: true, url: "/Logging/LoggingMessage/", data: { message: message }, success: function (data) {
            if (data == "Success") {
            }
        }
    });
}
//去掉两边空格
function lTrim(str) {
    if (str.charAt(0) == " ") {
        str = str.slice(1);
        str = lTrim(str);
    }
    return str;
}
function rTrim(str) {
    var iLength;
    iLength = str.length;
    if (str.charAt(iLength - 1) == " ") {
        str = str.slice(0, iLength - 1);
        str = str.substring(0, iLength - 1);
        str = rTrim(str);
    }
    return str;
}
function trim(str) {
    return lTrim(rTrim(str));

}

function DeleteComplete(urls) {
    window.location = urls;
}


function IsUrl(url) {
    return url.charAt(0) == '/';
}


$(function () {
    var showCookieName = "SHOW_INVITECODE_DIALOG";
    $("#tabContainer > ul > li").click(function () {
        var container = $("#tabContainer");
        var selectClass = container.attr("selectClass");
        if (selectClass == undefined) {
            selectClass = "uselect_";
        }
        var arrSelectClass = selectClass.split('_');
        var select = arrSelectClass[0];
        var unSelect = arrSelectClass[1];
        var accLi = $(this);
        accLi.addClass(select).removeClass(unSelect);
        accLi.siblings().addClass(unSelect).removeClass(select);;
        var accDiv = $("[type='container']").eq($("#tabContainer > ul > li").index(accLi));
        accDiv.show();
        accDiv.siblings("[type='container']").hide();
    });
    //控制textarea输入字数
    $("textarea[data-val-length-max]").on("keyup", function (event) {
        if (!(event.shiftKey && event.keyCode == 16) && !(event.altKey && event.keyCode == 18) && !(event.ctrlKey && event.keyCode == 17)) {
            var spWordNum = $(this).parent().find("span[tp='sp_caculateWordCount']");
            var maxCount = parseInt($(this).attr("data-val-length-max"));
            if (spWordNum) {
                var valText = $(this).val();
                var valTextArry = valText.split(new RegExp("\\n|\\r\\n", "igm"));
                var spaceWord = valTextArry.length - 1;
                if (!$.support.leadingWhitespace) { /* IE 6-8 */
                    switch (maxCount) {
                        case 200:
                            if (valText.length >= 180) spaceWord += 10;
                            break;
                        case 500:
                            if (valText.length >= 460) spaceWord += 20;
                            break;
                        case 2000:
                            if (valText.length >= 1900) spaceWord += 30;
                            break;
                    }
                }
                var remainNum = maxCount - (valText.length + spaceWord);
                var c = spWordNum.children("b.str-length-surplus").eq(0);
                if (remainNum < 0) {
                    c.text(0);
                    $(this).val(valText.substring(0, maxCount - spaceWord));
                } else {
                    c.text(remainNum);
                }
                $(this).valid();
            }
        }
    });

    //禁止文本框输入值两端的空格符
    $("input:text").on("blur", function () {
        $(this).val($.trim($(this).val()));
    });
    $('a[href^="http://"]').attr("target", "_blank");

    function alertCoreHandler(iconClass, msg, fun, option) {
        if ($('div[role="dialog"]').length > 0 && !$('div[role="dialog"]').is(':hidden')) return;
        var btnText = '确定', isHideIcon = false, closeIsActive = true;
        if (typeof option == 'object') {
            btnText = option.btnText || btnText; // 按钮文字
            isHideIcon = option.isHideIcon; // 是否隐藏图标
        }
        $.messageCallback = fun;
        var html = '';
        html += '<div class="dialog_bg div-dialog-bg"></div>';
        html += '<div class="dialog_alert div-dialog-content" style="width:auto;min-width:400px;z-index:9999;top:40.4567%">';
        html += '<div class="dialog_title">';
        html += '<p>温馨提示</p>';
        var caction = iconClass == 'dialog_w' ? '' : 'btn-dialog-close-action';
        html += '<a href="javascript:;" class="btn-dialog-close closebtn ' + caction + '">X</a></div>';
        html += '<div class="dialog_inner">';
        if (!isHideIcon) {
            html += '<p class="' + iconClass + '" id="dialog-msg-icon"><i></i>' + msg + '</p>';
        } else {
            html += '<p>' + msg + '</p>';
        }
        html += '</div><div class="dialog_bottom"><a href="javascript:;" class="btn-dialog-close btn-dialog-close-action">' + btnText + '</a></div></div>';
        $('body').append(html);
        var content = $('.div-dialog-content'),
            width = content.width() / 2;
        if (width === 0)width = 295;
        content.css({ 'marginLeft': -width });
        $('.btn-dialog-close').on('click', function () {
            $('.div-dialog-bg,.div-dialog-content').remove();
        })
        $('.btn-dialog-close-action').on('click', function () {
            if (typeof $.messageCallback == 'function') {
                $.messageCallback();
            }
        })
    }
    var zds = window.zds || {};
    zds.alert = function (msg, fun, option) {
        alertCoreHandler('dialog_s', msg, fun, option);
    }
    zds.warn = function (msg, fun, option) {
        alertCoreHandler('dialog_w', msg, fun, option);
    }
    zds.openEmail = function (email) {
        var addList = [
            { 'qq.com': 'https://mail.qq.com/cgi-bin/loginpage' },
            { '163.com': 'http://mail.163.com/' },
            { '126.com': 'http://mail.126.com/' },
            { 'sohu.com': 'http://mail.sohu.com/' },
            { 'sina.com': 'http://mail.sina.com.cn/' },
            { 'gmail.com': 'https://mail.google.com/' },
            { '21cn.com': 'http://mail.21cn.com/w2/' },
            { 'yahoo.com': 'https://login.yahoo.com' },
            { 'hotmail.com': 'https://login.live.com' },
            { 'vip.qq.com': 'https://mail.qq.com/cgi-bin/loginpage' },
            { 'yeah.net': 'http://www.yeah.net/' },
            { 'sogou.com': 'http://mail.sogou.com/' },
            { 'tom.com': 'http://web.mail.tom.com/webmail/login/index.action' },
            { 'eyou.com': 'http://www.eyou.com/' },
            { 'inbox.com': 'http://email.inbox.com/' },
            { 'live.com': 'https://login.live.com/' },
            { 'mail.com': 'http://www.mail.com/int/' },
            { '263.net': 'http://www.263.net/' }
        ];
        if (email.split('@').length != 2) return '邮箱格式不正确';
        var suffix = email.split('@')[1];
        for (var i = 0; i < addList.length; i++) {
            for (var n in addList[i]) {
                if (n == suffix) {
                    window.open(addList[i][n]);
                }
            }
        }
        zds.warn('请手动打开您的邮箱查收激活邮件')
    }
    $.ajaxSetup({
        headers: { 'X-Authorization': 'Authorization-Origins-Express' }
        //, xhrFields: {
        //    withCredentials: true
        //}
    });
    zds.invokeApi = function (method, baseUrl, relativeUrl, obj, callback) {
        var apiUrl = baseUrl + relativeUrl;
        if (method.toUpperCase() == 'POST') {
            $.post(apiUrl, obj, callback);
        } else {
            $.get(apiUrl, callback);
        }
    }
    zds.invokeShopApi = function (relativeUrl, data, callback) {
        var baseUrl = location.host.indexOf('localhost') > -1 ? zds.baseApiUrl : (location.host.indexOf('198.18.0.254') > -1 ? 'http://198.18.0.254:8093/api/' : 'http://jzapi.sumszw.com/api/');
        if (typeof data == 'object') {
            zds.invokeApi('POST', baseUrl, relativeUrl, data, callback);
        } else {
            zds.invokeApi('GET', baseUrl, relativeUrl, undefined, data);
        }
    }
    zds.baseApiUrl = 'http://198.18.0.254:8093/api/'; //'http://localhost:4560/Api/';
    zds.userName = $('#cookie_userName').val();
    zds.parseObject = function (array, filter) {
        var obj = {};
        for (var i = 0; i < array.length; i++) {
            if (this.isArray(filter) && filter.indexOf(array[i]['name']) > -1) {
                continue;
            }
            obj[array[i]['name']] = array[i]['value'];
        }
        return obj;
    }
    zds.setValueToForm = function (obj) {
        for (var n in obj) {
            var ele = $('#' + n);
            if (ele.length == 0) continue;
            if (n.toLowerCase().indexOf('image') > -1 && obj[n]) {
                ele.val(obj[n]);
                var imgs = obj[n].split(','), li = '';
                for (var i = 0; i < imgs.length; i++) {
                    if (imgs[i].length > 0) {
                        $('img.img-default').hide();
                        li += '<li><span>X</span><img src="' + imgs[i] + '" alt="" /></li>';
                    }
                }
                $('#image-ul-pic').html(li);
                continue;
            }
            ele.val(obj[n]);
        }
    }
    zds.isArray = function (o) {
        return Object.prototype.toString.call(o) === '[object Array]';
    }
    zds.exit = function () {
        $.post("/Login/Exit", function (data) {
            if (data == "OK") {
                window.location.href = "/login";
            }
        });
    }
    window.zds = zds;
});

//普通弹出
function ShowDialog(doc, title, height, width, closeFunc) {
    $(doc).dialog({
        title: title,
        closeOnEscape: false,
        resizable: false,
        modal: true,
        draggable: false,
        minHeight: 'auto',
        minWidth: width,
        close: closeFunc
    });
}
/*全选*/
function CheckAll(obj) {
    $("input[type='checkbox'][name='chkall']").each(function (ind, doc) {
        if (obj.checked) {
            doc.checked = true;
        } else {
            doc.checked = false;
        }
    });
}
/* add js utils function by:Ax0ne */
String.prototype.trim = function () { return this.replace(/^[\s\xA0]+/, "").replace(/[\s\xA0]+$/, "") }
String.prototype.format = function () {
    var args = arguments;
    return this.replace(/{(\d+)}/g, function (match, number) {
        return typeof args[number] != 'undefined' ? args[number] : match;
    });
};