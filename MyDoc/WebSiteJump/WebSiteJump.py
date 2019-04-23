
import my_xmlConfig
import requests
from requests import exceptions
import json
from time import sleep
from bs4 import BeautifulSoup
import logging
from logging.handlers import RotatingFileHandler
import re
import os
import stat
import base64
from concurrent.futures import ThreadPoolExecutor, wait, ALL_COMPLETED

class _SiteReponse:
    def __init__(self):
        self.text = ''      #
        self.url = ''       #

class _s_RequestPost_Data:
    def __init__(self):
        self.id = 0                 # 任务ID
        self.SuccessCount = 0       # 成功数量
        self.FailCount = 0          # 失败数量
        self.KZPathList = []        # 待检验的快照地址
        self.Failsitelist = []      # 失败网站列表

    def ReSet(self):
        self.id = 0
        self.SuccessCount = 0
        self.FailCount = 0
        self.KZPathList.clear()
        self.Failsitelist.clear()



g_sTaskData = _s_RequestPost_Data()

websiteLists = []
hotWordList = []
classifyList = []
#mutex = threading.Lock()
#读取配置
configInfo = my_xmlConfig.getConfig('config.xml')
#创建一个logger
logger = logging.getLogger('WebSiteJump_Log')
#fh = logging.FileHandler('Collectkeyword.Log', encoding='UTF-8')


#0:info 1:error
def printLog(strInfo1, strInfo2 = '', level = 0):
    #mutex.acquire()
    loginfo = strInfo1
    loginfo += strInfo2

    if level == 0:
        logger.info(loginfo)
    else:
        logger.error(loginfo)
    #mutex.release()


def InitLogger():
    #mutex.acquire()
    logger.setLevel(logging.DEBUG)
    #创建一个handler,用于写入日志文件
    fh = RotatingFileHandler('WebSiteJump.Log', encoding='UTF-8', maxBytes=3*1024*1024, backupCount=10)
    os.chmod('WebSiteJump.Log', stat.S_IRWXO + stat.S_IRWXG + stat.S_IRWXU)
    #fh = logging.RotatingFileHandler('Collectkeyword.Log', encoding='UTF-8', maxBytes=1*1024, backupCount=3)
    #fh.close()
    fh.setLevel(logging.DEBUG)
    #定义handler的输出格式
    #formatter = logging.Formatter('%(asctime)s - %(levelname)s - %(filename)s[:%(lineno)d] - %(message)s')
    formatter = logging.Formatter('%(thread)d - %(message)s')
    fh.setFormatter(formatter)
    logger.addHandler(fh)
    #mutex.release()

def Get_ByRequests(strUrl):
    bCallOk = False
    myheaders = {
        'user-agent': 'Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/70.0.3538.110 Safari/537.36 SumTest',
    }
    #printLog('网址：', strUrl)

    for index in range(0, 2):
        try:
            bCallOk = True
            requests.packages.urllib3.disable_warnings()
            response = requests.get(strUrl, headers=myheaders, verify=False)
            #如果状态不是200，引发HttpError异常
            response.raise_for_status()
            response.encoding = response.apparent_encoding
            break
        except exceptions.Timeout as e:
            print(e)
            printLog('Timeout', '', 1)
            bCallOk = False
        except exceptions.HTTPError as e:
            print(e)
            printLog('HTTPError', '', 1)
            bCallOk = False
        except:
            print('访问异常%s'%strUrl)
            printLog('访问异常：', strUrl)
            bCallOk = False

    if bCallOk:
        sitePeponsr = _SiteReponse()
        sitePeponsr.text = response.text
        sitePeponsr.url = response.url
        return sitePeponsr
    else:
        return ''


def GetHtmlType(_strUrl):
    strUrl = _strUrl.lower()
    if strUrl.find('_baidu') != -1:
        return 0
    elif strUrl.find('_phonebaidu') != -1:
        return 1
    elif strUrl.find('_360') != -1:
        return 2
    elif strUrl.find('_phone360') != -1:
        return 3
    elif strUrl.find('_sogou') != -1:
        return 4
    elif strUrl.find('_phonesogou') != -1:
        return 5
    elif strUrl.find('_bing') != -1:
        return 6
    else:
        return -1

def GetLocation(_webText):
    webText = _webText
    iTextLen = len(webText)

    if iTextLen < 5:
        return ''

    elif iTextLen < 1000:
        bWebSite = False
        webTextTemp = ''
        try:
            webTextTemp = webText.lower()
        except:
            webTextTemp = webText
        if webTextTemp.find("window.location.replace") != -1:
            iTargetChar = webTextTemp.find("window.location.replace")
            strUrl = webTextTemp[iTargetChar:]
            strData = webText[iTargetChar:]
            bFind = True
        elif webTextTemp.find("<meta http-equiv") != -1:
            iTargetChar = webTextTemp.Find("<meta http-equiv")
            strUrl = webTextTemp[iTargetChar:]
            strData = webText[iTargetChar:]
            bFind = True
        elif webTextTemp.find("http:") != -1 or webTextTemp.find("https:") != -1:
            bFind = True
            strUrl = webTextTemp


        if bFind == True:
            iPosUrl = strUrl.find("http:")
            if -1 == iPosUrl:
                iPosUrl = strUrl.find("https:")
            if -1 != iPosUrl:
                strUrl = strUrl[iPosUrl:]
                strData = strData[iPosUrl:]
                index = 0
                for char in strUrl:
                    #TCHAR cChar = strUrlA.GetAt(index)
                    cChar = char
                    index += 1
                    if ' ' == cChar or '>' == cChar or '\"' == cChar or '\'' == cChar or '\r' == cChar or '\n' == cChar:
                        strData = strData[:index - 1]
                        strUrl = strData
                        return strUrl
        return ''


def GetHotWordMethon(strUrl, strXPath):
    webText = Get_ByRequests(strUrl).text
    if webText == '':
        printLog('数据为空：', strUrl)
        return

    soup = BeautifulSoup(webText, "lxml")
    JumpOk = True
    try:
        div_id_sum_list = soup.find_all('div', id=re.compile(r'^sum4'))
        iHtmlType = GetHtmlType(strUrl)
        for div_id_sum in div_id_sum_list:
            try:
                tagAttrs = div_id_sum['style']
            except:
                continue
            if tagAttrs.find('rgb(255') != -1:
                if iHtmlType == 0:
                    #百度
                    sHref = div_id_sum.h3.a['href']
                elif iHtmlType == 1:
                    #手机百度
                    sHref = div_id_sum.header.div.a['href']
                elif iHtmlType == 2:
                    #360
                    sHref = div_id_sum.h3.a['href']
                elif iHtmlType == 3:
                    #手机360
                    sHref = div_id_sum.a['href']
                elif iHtmlType == 4:
                    #搜狗
                    sHref = div_id_sum.h3.a['href']
                elif iHtmlType == 5:
                    #手机搜狗
                    sHref = div_id_sum.a['href']
                elif iHtmlType == 6:
                    #必应
                    sHref = div_id_sum.h2.a['href']
                else:
                    continue

                SiteInfo = Get_ByRequests(sHref)
                sTextTemp = SiteInfo.text
                if len(sTextTemp) < 2000:
                    #location = re.findall(r"http?://[^, \"]*?\.(?:html)", sTextTemp)
                    location = GetLocation(sTextTemp)
                    if len(location) > 0:
                        sTextTemp = Get_ByRequests(location).text

                logger.info(strUrl)
                sJs = re.findall(r"staticimages1.oss-cn-shenzhen.aliyuncs.com/js[^\"]*&amp;", sTextTemp)
                sBase64Url = ''
                if len(sJs) > 0:
                    #https://staticimages1.oss-cn-shenzhen.aliyuncs.com/js/20190419/_0419_zc_878a.js?id=d3dmL3bltlxGgjbmL4bvN==Q&amp;
                    sBase64Url = sJs[0]
                    ipos1 = sBase64Url.find('=')
                    ipos2 = sBase64Url.find('&amp')
                    sBase64Url = sBase64Url[ipos1+1:ipos2]
                    sBase64Url = sBase64Url.strip()
                elif sTextTemp.find('hide_FLAG_for_qkphoto') != -1 or sTextTemp.find('hide_FLAG_for_qkphoto') != -1:
                    #商铺
                    logger.info('ok')
                    continue
                else:
                    #找不到js，肯定不会跳转
                    logger.error('Fail, ErrorCode=1001')
                    JumpOk = False
                    continue

                slist = list(sBase64Url)
                slen = len(slist)

                for index in range(0, slen, 3):
                    if slen - index >= 3:
                        sfirst = slist[index]
                        sthree = slist[index + 2]
                        slist[index] = sthree
                        slist[index + 2] = sfirst
                sBase64Url = ''.join(slist)
                try:
                    sJumpUrl = base64.b64decode(sBase64Url)
                    logger.info('ok')
                except:
                    #解码失败，当做跳转失败
                    print('base64解码异常：%s' % strUrl)
                    logger.error('Fail, ErrorCode=1002')
                    JumpOk = False
    except:
        print('异常：%s'%strUrl)
        JumpOk = False

    if JumpOk == True:
        g_sTaskData.SuccessCount += 1
    else:
        g_sTaskData.FailCount += 1
        g_sTaskData.Failsitelist.append(strUrl)




def post_data(id, sKeyWord):
    hotWordLists = list(set(hotWordList))  # 热词去重
    classifyLists = list(set(classifyList))  # 类目去重

    HotWordList = ''
    ClassifyList = ''
    for hotword in hotWordLists:
        if HotWordList == '':
            HotWordList += hotword
        else:
            HotWordList += '@$_$@'
            HotWordList += hotword

    for classify in classifyLists:
        if ClassifyList == '':
            ClassifyList += classify
        else:
            ClassifyList += '@$_$@'
            ClassifyList += classify

    #url = 'http://192.168.1.83:1702/api/KeywordCollect/SaveKeywordCollectResult'
    url = configInfo.url_reponse
    body = {}
    body["Id"] = id
    body["Keyword"] = sKeyWord
    body["HotKeywordList"] = HotWordList
    body["SubjectList"] = ClassifyList
    headers = {'content-type': "application/json"}

    printLog('待提交数据：')
    #mutex.acquire()
    logger.info(body)
    #mutex.release()

    for index in range(0, 2):
        try:
            response = requests.post(url, data=json.dumps(body), headers=headers)
            response.encoding = response.apparent_encoding
            statusCode = response.status_code
            if statusCode == 200:
                # '{"Data":true,"Elapsed":576,"IsException":false,"Message":"Api一路通顺（^_^）"}'
                text = response.text

                printLog('服务器返回的数据：', text)
                try:
                    hjson = json.loads(text)
                    reValue = hjson['Data']
                    if reValue == True:
                        #mutex.acquire()
                        logger.info('提交采集数据成功')
                        #mutex.release()
                        break
                except:
                    sleep(2)
                    continue
            else:
                printLog('提交数据失败,服务器返回的状态码：', str(statusCode), 1)
                sleep(2)
                continue
        except Exception as e:
            printLog('提交数据失败', url, 1)
            sleep(2)
            continue

def log_backup(filePath):
    #mutex.acquire()
    #try:
    if os.path.isfile(filePath):
        fsize = os.path.getsize(filePath)
        fsize = fsize/float(1024*1024)
        if round(fsize, 3) > 0.003:
            ipos = filePath.find('.')
            if ipos > 0:
                #fh.close()
                #fh = logging.FileHandler('Collectkeyword.Log', encoding='UTF-8')
                #fh.close()
                dstDir = filePath.replace('.', '_old.')
                if os.path.isfile(dstDir):
                    os.remove(dstDir)
                os.rename(filePath, dstDir)
                InitLogger()
    #except:
    #    logger.error('日志备份异常')
    #mutex.release()


def JopRun(_KZPathList):
    for websiteUrl in _KZPathList:
        if websiteUrl.find('http') != -1:
            GetHotWordMethon(websiteUrl, '')


def main():

    InitLogger()
    printLog('**************************程序开启**************************\r\n\r\n')
    # 实例化对象， 线程池包含10个线程来处理任务;
    pool = ThreadPoolExecutor(max_workers=5)

    print('程序启动，开始网页跳转测试，测试信息见日志文件WebSiteJump.Log')
    while 1:
        print('请求任务')
        g_sTaskData.ReSet()
        websiteLists.clear()

        #请求任务
        bRequestOk = False

        '''
        url = 'http://kz-snapshot.oss-cn-shenzhen.aliyuncs.com/%E9%9C%B8%E5%B7%9E%E5%B8%82%E5%BA%B7%E4%BB%99%E5%BA%84%E5%8C%97%E6%'
        #url = configInfo.url_request
        r_data = Get_ByRequests(url).text
        if r_data == '':
            print('没请求到任务，程序睡眠%d秒钟'%5)
            printLog('没请求到任务，程序睡眠')
            sleep(int(configInfo.intervaltime))
            continue
        '''

        sUrl1 = 'http://kz-snapshot.oss-cn-shenzhen.aliyuncs.com/2023/泰州市佳德汽车配件有限公司/房车外推窗_泰州市佳德汽车配件有限公司_BAIDU.html'
        sUrl2 = 'http://kz-snapshot.oss-cn-shenzhen.aliyuncs.com/2123/泰州市佳德汽车配件有限公司/改装车铝窗_泰州市佳德汽车配件有限公司_PHONEBAIDU.html'
        sUrl3 = 'http://kz-snapshot.oss-cn-shenzhen.aliyuncs.com/1923/泰州市佳德汽车配件有限公司/房车外推窗_泰州市佳德汽车配件有限公司_360.html'
        sUrl4 = 'http://kz-snapshot.oss-cn-shenzhen.aliyuncs.com/广州猎人气网络科技有限公司/猎人气流量_广州猎人气网络科技有限公司_PHONE360.html'
        sUrl5 = 'http://kz-snapshot.oss-cn-shenzhen.aliyuncs.com/山东星月网络科技有限公司/太原分销商城价格_山东星月网络科技有限公司_SOGOU.html'
        sUrl6 = 'http://kz-snapshot.oss-cn-shenzhen.aliyuncs.com/聊城市永上钢管有限公司/一次性隧道钢花管价格_聊城市永上钢管有限公司_PHONESOGOU.html'
        sUrl7 = 'http://kz-snapshot.oss-cn-shenzhen.aliyuncs.com/2223/广州圣蜜科技有限公司/黄金艾宝草本液联系电话是多少_广州圣蜜科技有限公司_BING.html'
        for index in range(0, 6):
            g_sTaskData.KZPathList.append(sUrl1)
            g_sTaskData.KZPathList.append(sUrl2)
            g_sTaskData.KZPathList.append(sUrl3)
            g_sTaskData.KZPathList.append(sUrl4)
            g_sTaskData.KZPathList.append(sUrl5)
            g_sTaskData.KZPathList.append(sUrl6)
            g_sTaskData.KZPathList.append(sUrl7)

        #获取任务总数
        iAllTaskNum = len(g_sTaskData.KZPathList)
        kzTaskLists = []
        if iAllTaskNum > 0:
            kzTaskList = []
            iCount = 0
            for kzPath in g_sTaskData.KZPathList:
                kzTaskList.append(kzPath)
                iCount += 1
                if iCount == 5:
                    #线程每次处理5个快照
                    kzTaskLists.append(kzTaskList)
                    kzTaskList = []
                    iCount = 0
            if iCount > 0 and iCount < 5:
                if len(kzTaskList) > 0:
                    kzTaskLists.append(kzTaskList)

        #将任务全部添加到线程池
        all_task = [pool.submit(JopRun, kzTask) for kzTask in kzTaskLists]

        while 1:
            sleep(3)
            iDownNum = (g_sTaskData.SuccessCount + g_sTaskData.FailCount)*100 / iAllTaskNum
            print('任务进行中，已完成%.2f%%'%iDownNum)
            if iDownNum >= 100:
                break

        wait(all_task, return_when=ALL_COMPLETED)
        print('任务完成\r\n')

main()



