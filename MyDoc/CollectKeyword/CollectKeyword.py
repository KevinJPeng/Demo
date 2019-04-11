
import my_xmlConfig
import requests
from requests import exceptions
import json
from time import sleep
import requests
from bs4 import BeautifulSoup
from lxml import etree
from selenium import webdriver
from selenium.webdriver.chrome.options import Options
import urllib
import logging
import re

hotWordList = []
classifyList = []

#读取配置
configInfo = my_xmlConfig.getConfig('config.xml')
#创建一个logger
logger = logging.getLogger('Collectkeyword_Log')


#0:info 1:error
def printLog(strInfo1, strInfo2 = '', level = 0, iType = 1):
    loginfo = strInfo1
    if iType == 0:
        for info in strInfo2:
            loginfo += info
            loginfo += '、'
    else:
        loginfo += strInfo2

    if level == 0:
        logger.info(loginfo)
    else:
        logger.error(loginfo)



def InitLogger():
    logger.setLevel(logging.DEBUG)
    #创建一个handler,用于写入日志文件
    fh = logging.FileHandler('Collectkeyword.Log', encoding='UTF-8')
    fh.setLevel(logging.DEBUG)
    #定义handler的输出格式
    formatter = logging.Formatter('%(asctime)s - %(levelname)s - %(filename)s[:%(lineno)d] - %(message)s')
    fh.setFormatter(formatter)
    logger.addHandler(fh)



def GetUrl_ByWebdriver(url, num_retries = 2):
    hotWordList_temp = []
    classifyList_temp = []
    hotWordList_temp.clear()
    classifyList_temp.clear()

    printLog('网址：', url)
    bRequestOk = False
    for index in range(0, 2):
        try:
            chrome_options = Options()
            # 设置chrome浏览器无界面模式
            chrome_options.add_argument('--headless')
            browser = webdriver.Chrome(options=chrome_options)
            # browser = webdriver.Chrome()
            browser.get(url)
            # 休眠3秒,等待加载完成!
            sleep(3)
            bRequestOk = True
            break
        except Exception as e:
            #print('open browser error:%s'%e)
            printLog('Open browser error', '', 1)

    if bRequestOk:
        #采集热词
        #keywordlist = []
        try:
            keywordlist = browser.find_element_by_class_name('s-filter-suggest').find_element_by_class_name('s-suggest-wrap').find_elements_by_class_name('sj-suggest-item')
            for keywordsingle in keywordlist:
                print(keywordsingle.text)
                hotWordList.append(keywordsingle.text)
                hotWordList_temp.append(keywordsingle.text)
        except Exception as e:
            #print('get hotWord error:%s'%e)
            printLog('Get hotWord error', '', 1)

        # 采集分类
        try:
            keywordlist = browser.find_element_by_class_name('s-filter-suggest').find_element_by_class_name('s-filter-wrap').find_elements_by_tag_name('tr')
            keywordlist = keywordlist[len(keywordlist) - 1]
            keywordlist = keywordlist.find_element_by_class_name('sj-filter-word').find_elements_by_tag_name('span')
            for keywordsingle in keywordlist:
                print(keywordsingle.text)
                classifyList.append(keywordsingle.text)
                classifyList_temp.append(keywordsingle.text)
        except Exception as e:
            #print('get classifyWord error:%s'%e)
            printLog('Get classifyWord error', '', 1)

    try:
        # 关闭浏览器
        browser.close()
        # 关闭chreomedriver进程
        browser.quit()
    except Exception as e:
        #print('close browser error:%s' % e)
        printLog('Close browser error', '', 1)

    printLog('热词：', hotWordList_temp, 0, 0)
    printLog('分类：', classifyList_temp, 0, 0)

    #return hotWordList


def GetUrl_ByRequests(strUrl):
    bCallOk = False
    myheaders = {
        'user-agent': 'Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/70.0.3538.110 Safari/537.36',
        #'Accept-Encoding': 'gzip, deflate'
    }
    printLog('网址：', strUrl)

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
        return response.text
    else:
        return ''


def GetHotWordMethon0(strUrl, strXPath):
    hotWordList = GetUrl_ByWebdriver(strUrl, 2)

    return hotWordList

def GetHotWordMethon1(strUrl, strXPath):
    hotWordList_temp = []
    classifyList_temp = []
    hotWordList_temp.clear()
    classifyList_temp.clear()

    webText = GetUrl_ByRequests(strUrl)
    if webText == '':
        printLog('数据为空：', strUrl)
        return

    #采集热词
    bFindOk = False
    soup = BeautifulSoup(webText, "lxml")
    try:
        # 查找所有class属性为hd的div标签
        div_des = soup.find('div', id='propertySearch')
        div01 = div_des.find('div', class_='filter-panel')
        div02 = div01.find_all('dl', class_='clear')
        test01 = div02[len(div02) - 1].dd.ul.find_all('li', class_='filter-attr-item')
    except AttributeError as e:
        print(e)
        printLog('hotWord AttributeError', '', 1)
    else:
        if test01 == None:
            print('没有找到目的标签')
            printLog('hotWord没有找到目的标签：', strUrl)
        else:
            bFindOk = True

    if bFindOk:
        for test001 in test01:
            print(test001.text)
            hotWordList.append(test001.text)
            hotWordList_temp.append(test001.text)
    #else:
        #hotWordList = ''


    #采集分类
    bFindOk = False
    try:
        # 查找所有class属性为hd的div标签
        div_des = soup.find('div', id='propertySearch')
        div01 = div_des.find('div', class_='filter-panel')
        div02 = div01.find_all('dl', class_='clear')
        test01 = div02[0].dd.ul.find_all('li')
    except AttributeError as e:
        print(e)
        printLog('classify AttributeError', '', 1)
    else:
        if test01 == None:
            print('没有找到你想要的标签')
            printLog('classify没有找到目的标签：', strUrl)
        else:
            bFindOk = True

    if bFindOk:
        for test001 in test01:
            print(test001.text)
            classifyList.append(test001.text)
            classifyList_temp.append(test001.text)
    #else:
        #classifyList = ''

    printLog('热词：', hotWordList_temp, 0, 0)
    printLog('分类：', classifyList_temp, 0, 0)


    #return hotWordList

def GetHotWordMethon2(strUrl, strXPath):
    hotWordList_temp = []
    classifyList_temp = []
    hotWordList_temp.clear()
    classifyList_temp.clear()

    webText = GetUrl_ByRequests(strUrl)
    if webText == '':
        printLog('数据为空：', strUrl)
        return

    #采集热词
    bFindOk = False
    soup = BeautifulSoup(webText, "lxml")
    try:
        # 查找所有class属性为hd的div标签
        div_des = soup.find('div', id='sm-maindata')
        div01 = div_des.find('div', class_='sm-related s-module-related').find('div', class_='s-widget-related')
        div02 = div01.find_all('div')
        test01 = div02[len(div02) - 1].ul.find_all('li')
    except AttributeError as e:
        print(e)
        printLog('hotWord没有找到目的标签：', strUrl)
    else:
        if test01 == None:
            print('没有找到你想要的标签')
            printLog('hotWord没有找到目的标签：', strUrl)
        else:
            bFindOk = True

    if bFindOk:
        for test001 in test01:
            print(test001.span.text)
            hotWordList.append(test001.span.text)
            hotWordList_temp.append(test001.span.text)


    #采集分类
    bFindOk = False
    try:
        # 查找所有class属性为hd的div标签
        div_des = soup.find('div', id='sm-maindata')
        div01 = div_des.find('div', class_=re.compile(r'^sm-sn')).find('div', class_=re.compile(r'^s-widget-flatcat'))
        #div01 = div_des.find('div', class_='sm-sn s-module-sn').find('div', class_='s-widget-flatcat sm-widget-row sm-sn-items-control sm-sn-items-count-d fd-clr')
        div02 = div01.find_all('div')
        test01 = div02[len(div02) - 1].ul.find_all('li')
    except AttributeError as e:
        print(e)
        printLog('classify AttributeError', '', 1)
    else:
        if test01 == None:
            print('没有找到你想要的标签')
            printLog('classify没有找到目的标签：', strUrl)
        else:
            bFindOk = True

    if bFindOk:
        for test001 in test01:
            print(test001.span.text)
            classifyList.append(test001.span.text)
            classifyList_temp.append(test001.span.text)

    printLog('热词：', hotWordList_temp, 0, 0)
    printLog('分类：', classifyList_temp, 0, 0)


def GetHotWord(s_configInfo, _keyword):
    #hotWordList = []
    configInfo = s_configInfo
    for siteInfo in configInfo.sitelist:
        if siteInfo.siteDict['enable'] == '1':      #网站是否启用
            methonType = siteInfo.siteDict['methon']
            url = siteInfo.siteDict['url']
            if methonType == '0':
                utf_gb2312 = _keyword.encode('utf-8')
                url_code_name = urllib.parse.quote(utf_gb2312)
                url = 'https://b2b.baidu.com/s?q={}'.format(url_code_name)
                #url = 'https://b2b.baidu.com/s?q=手机壳'
                GetHotWordMethon0(url, '')
            elif methonType == '1':
                utf_gb2312 = _keyword.encode('utf-8')
                url_code_name = urllib.parse.quote(utf_gb2312)
                url = 'http://www.abiz.com/products/search?key={}'.format(url_code_name)
                #url = 'http://www.abiz.com/products/search?key=%E6%89%8B%E6%9C%BA%E5%A3%B3'
                GetHotWordMethon1(url, '')
            elif methonType == '2':
                try:
                    utf_gb2312 = _keyword.encode('gbk')
                    url_code_name = urllib.parse.quote(utf_gb2312)
                except:
                    logger.error('关键词转码失败')
                    continue
                url = 'https://s.1688.com/selloffer/offer_search.htm?keywords={}'.format(url_code_name)
                #url = 'https://s.1688.com/selloffer/offer_search.htm?keywords=%CA%D6%BB%FA%BF%C7'
                #url = 'https://s.1688.com'
                GetHotWordMethon2(url, '')
            else:
                print('无效方法')
                printLog('无效方法：methonType = ', str(methonType))
    #return hotWordList

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
    logger.info(body)

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
                        logger.info('提交采集数据成功')
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

def main():

    InitLogger()
    while 1:
        printLog('**************************我是分割线**************************\r\n\r\n')
        hotWordList.clear()
        classifyList.clear()

        #请求任务
        bRequestOk = False

        #url = 'http://192.168.1.83:1702/api/KeywordCollect/GetKeywordCollectTask'
        url = configInfo.url_request
        r_data = GetUrl_ByRequests(url)
        if r_data == '':
            print('没请求到任务，程序睡眠%s秒钟'%(configInfo.intervaltime))
            printLog('没请求到任务，程序睡眠')
            sleep(int(configInfo.intervaltime))
            continue

        #r_data = '{"Data":{"UserId":109046,"ProductId":146228,"Keyword":"香蕉","State":1,"UpdateTime":"2019-04-10 14:08:37","Remark":"任务被请求","AddTime":"2019-04-04 09:55:40","AddMan":"","Id":16},"Elapsed":57,"IsException":false,"Message":"Api一路通顺（^_^）"}'
        print('任务数据%s'%r_data)
        printLog('任务数据：', r_data)
        try:
            hjson = json.loads(r_data)
            Id = hjson['Data']['Id']
            Keyword = hjson['Data']['Keyword']
            if Id == '' or Keyword == '':  # 判断是否请求到任务
                print('json解析失败，程序睡眠%s秒钟' % (configInfo.intervaltime))
                printLog('json解析失败，程序睡眠')
                sleep(int(configInfo.intervaltime))
                continue
        except Exception as e:
            printLog('json解析异常')
            sleep(int(configInfo.intervaltime))
            continue

        GetHotWord(configInfo, Keyword)
        post_data(Id, Keyword)

main()



