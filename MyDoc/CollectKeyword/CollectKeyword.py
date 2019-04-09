
import my_xmlConfig
import requests
from requests import exceptions
import json
from time import sleep
import requests
from bs4 import BeautifulSoup
from lxml import etree

hotWordList = []
configInfo = my_xmlConfig.getConfig('config.xml')

def GetHotWordMethon0(strUrl, strXPath):
    from selenium import webdriver
    from selenium.webdriver.chrome.options import Options

    hotWordList = []

    chrome_options = Options()
    # 设置chrome浏览器无界面模式
    chrome_options.add_argument('--headless')
    browser = webdriver.Chrome(chrome_options=chrome_options)
    # browser = webdriver.Chrome()

    browser.get(strUrl)
    # print(browser.page_source)
    keywordlist = browser.find_element_by_class_name('s-filter-suggest').find_element_by_class_name('s-suggest-wrap').find_elements_by_class_name('sj-suggest-item')
    for keywordsingle in keywordlist:
        print(keywordsingle.text)
        hotWordList.append(keywordsingle.text)
    # 关闭浏览器
    browser.close()
    # 关闭chreomedriver进程
    browser.quit()
    return hotWordList

def CallUrl(strUrl):
    bCallOk = False
    myheaders = {
        'user-agent': 'Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/70.0.3538.110 Safari/537.36',
        #'Accept-Encoding': 'gzip, deflate'
    }

    for index in range(0, 2):
        try:
            bCallOk = True
            response = requests.get(strUrl, headers=myheaders, verify=False)
            #如果状态不是200，引发HttpError异常
            response.raise_for_status()
            response.encoding = response.apparent_encoding
            break
        except exceptions.Timeout as e:
            print(e)
            bCallOk = False
        except exceptions.HTTPError as e:
            print(e)
            bCallOk = False
        except:
            print('访问异常%s'%strUrl)
            bCallOk = False

    if bCallOk:
        return response.text
    else:
        return ''



def GetHotWordMethon1(strUrl, strXPath):
    hotWordList = []

    webText = CallUrl(strUrl)
    if webText == '':
        return hotWordList

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
    else:
        if test01 == None:
            print('没有找到你想要的标签')
        else:
            print(test01)
            bFindOk = True

    if bFindOk:
        for test001 in test01:
            print(test001.text)
            hotWordList.append(test001.text)
    else:
        hotWordList = ''

    return hotWordList

def GetHotWordMethon2(strUrl, strXPath):
    hotWordList = []

    webText = CallUrl(strUrl)
    if webText == '':
        return hotWordList

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
    else:
        if test01 == None:
            print('没有找到你想要的标签')
        else:
            print(test01)
            bFindOk = True

    if bFindOk:
        for test001 in test01:
            print(test001.span.text)
            hotWordList.append(test001.span.text)
    else:
        hotWordList = ''

    return hotWordList


def GetHotWord(s_configInfo):
    hotWordList = []
    configInfo = s_configInfo
    for siteInfo in configInfo.sitelist:
        if siteInfo.siteDict['enable'] == '1':      #网站是否启用
            methonType = siteInfo.siteDict['methon']
            url = siteInfo.siteDict['url']
            if methonType == '0':
                url = 'https://b2b.baidu.com/s?q=手机壳'
                hotWordList = GetHotWordMethon0(url, '')
            elif methonType == '1':
                url = 'http://www.abiz.com/products/search?key=%E6%89%8B%E6%9C%BA%E5%A3%B3'
                hotWordList += GetHotWordMethon1(url, '')
            elif methonType == '2':
                url = 'https://s.1688.com/selloffer/offer_search.htm?keywords=%CA%D6%BB%FA%BF%C7'
                #url = 'https://s.1688.com'
                hotWordList += GetHotWordMethon2(url, '')
            else:
                print('无效方法')
    return hotWordList

while 1:
    #请求任务
    bRequestOk = False
    for index in range(0, 3):
        r = requests.get('http://47.107.165.10:7001/home/getconfig')
        hjson = json.loads(r.text)
        if hjson['data']['endpoint'] != '':  #判断是否请求到任务
            bRequestOk = True
            break

    if bRequestOk:
        print('成功请求到任务')
        hotWordList = GetHotWord(configInfo)
    else:
        print('连续三次都没请求到任务，程序睡眠%s秒钟'%(configInfo.intervaltime))
        sleep(int(configInfo.intervaltime))
    hotWordList = list(set(hotWordList))        #热词去重
    print(hotWordList)




