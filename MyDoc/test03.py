import requests
import re
from bs4 import BeautifulSoup
from lxml import etree

#百度
# 通过find定位标签
# BeautifulSoup文档：https://www.crummy.com/software/BeautifulSoup/bs4/doc/index.zh.html
def bs_parse_movies(html):
    url_list = []
    site_list = []

    soup = BeautifulSoup(html)
    # 查找所有class属性为hd的div标签
    #div_list = soup.find_all('div', attrs={"id":re.compile(r"[1-9]?\d?$|30$")})
    div_list = soup.find_all('div', attrs={"id":re.compile(r"(^[1-2]?\d|30)$")})
    # 获取每个div中的a中的span（第一个），并获取其文本
    bFlag = 0
    for each in div_list:
        bFlag = 0
        a_list = each.find_all('a')
        #if a = 0:
        for aay in a_list:
            nn = aay.parent.name
            if nn== 'h3':
                if aay.attrs['href'].strip() != '':
                    url_list.append(aay.attrs['href'])

            try:
                cl = aay.attrs['class']
                if len(cl) > 0:
                    for cc in cl:
                        if cc == 'c-showurl':
                            span_list = aay.find_all('span')
                            if len(span_list) > 0:
                                for aas in span_list:
                                    nn = aas.parent
                                    if nn == aay:
                                        if aas.text.strip() != '':
                                            site_list.append(aas.text)
                                            bFlag = 1
                                            break
                            else:
                                if aay.text.strip() != '':
                                    site_list.append(aay.text)
                                    bFlag = 1
                                    break
                        if bFlag == 1:
                            break
                if bFlag == 1:
                    break
            except:
                print("error")
        if bFlag == 0:
            a_list = each.find_all('span')
            for aay in a_list:
                try:
                    c2 = aay.attrs['class']
                    for bb in c2:
                        if bb == 'c-showurl':
                            if aay.text.strip() != '':
                                site_list.append(aay.text)
                                bFlag = 1
                                break
                    if bFlag == 1:
                        break
                except:
                    print("error")

    count = 0
    for p1 in url_list:
        count += 1
        print(count, p1)
    count = 0
    print('\r\n')
    for p2 in site_list:
        count += 1
        print(count, p2)
    return url_list

# 360
# css选择器定位标签
# 更多ccs选择器语法：http://www.w3school.com.cn/cssref/css_selectors.asp
# 注意：BeautifulSoup并不是每个语法都支持
def bs_css_parse_movies(html):
    url_list = []
    site_list = []

    soup = BeautifulSoup(html)
    # 查找所有class属性为hd的div标签
    #div_list = soup.find_all('div', attrs={"id":re.compile(r"[1-9]?\d?$|30$")})
    div_list = soup.find_all('li', class_='res-list')
    # 获取每个div中的a中的span（第一个），并获取其文本
    bFlag = 0
    for each in div_list:
        bFlag = 0
        a_list = each.find_all('a')
        #if a = 0:
        for aay in a_list:
            nn = aay.parent.name
            if nn== 'h3':
                if aay.attrs['href'].strip() != '':
                    url_list.append(aay.attrs['href'])

            try:
                cl = aay.attrs['class']
                if len(cl) > 0:
                    for cc in cl:
                        if cc == 'c-showurl':
                            span_list = aay.find_all('span')
                            if len(span_list) > 0:
                                for aas in span_list:
                                    nn = aas.parent
                                    if nn == aay:
                                        if aas.text.strip() != '':
                                            site_list.append(aas.text)
                                            bFlag = 1
                                            break
                            else:
                                if aay.text.strip() != '':
                                    site_list.append(aay.text)
                                    bFlag = 1
                                    break
                        if bFlag == 1:
                            break
                if bFlag == 1:
                    break
            except:
                print("error")
        if bFlag == 0:
            a_list = each.find_all('cite')
            for aay in a_list:
                try:
                    if aay.text.strip() != '':
                        site_list.append(aay.text)
                        bFlag = 1
                        break
                except:
                    print("error")

    count = 0
    for p1 in url_list:
        count += 1
        print(count, p1)
    count = 0
    print('\r\n')
    for p2 in site_list:
        count += 1
        print(count, p2)
    return url_list


# XPATH定位标签
# 更多xpath语法：https://blog.csdn.net/gongbing798930123/article/details/78955597
def xpath_parse_movies(html):
    et_html = etree.HTML(html)
    # 查找所有class属性为hd的div标签下的a标签的第一个span标签
    urls = et_html.xpath("//div[@class='hd']/a/span[1]")

    movie_list = []
    # 获取每个span的文本
    for each in urls:
        movie = each.text.strip()
        movie_list.append(movie)

    return movie_list


def get_movies():
    headers = {
        'user-agent': 'Mozilla/5.0 (Windows NT 6.1; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/52.0.2743.82 Safari/537.36',
    }

    #link = 'http://www.baidu.com/s?wd=洛阳超滤膜价格&amp;pn=1'
    link = 'http://www.so.com/s?q=洛阳超滤膜价格&amp;pn=1'
    r = requests.get(link, headers=headers, timeout=10)
    print("响应状态码:", r.status_code)
    if 200 != r.status_code:
        return None

    # 三种定位元素的方式：

    # 普通BeautifulSoup find
    #return bs_parse_movies(r.text)
    # BeautifulSoup css select
    return bs_css_parse_movies(r.text)
    # xpath
    #return xpath_parse_movies(r.text)


movies = get_movies()
#print(movies)

