from xml.dom.minidom import parse
import xml.dom.minidom


class _s_configInfo:
    def __init__(self):
        self.intervaltime = ''       # 间隔时间
        self.url_request = ''       # 请求url
        self.url_reponse = ''       # 响应url
        self.sitelist = []          # 网站信息列表

class _s_siteInfo:
    def __init__(self):
        self.siteDict = {}          # 网站信息

def getConfig(fileName):
    #打开xml文档
    dom = xml.dom.minidom.parse(fileName)
    #得到文档元素对象
    root = dom.documentElement

    configInfo = _s_configInfo()
    intervaltime = root.getElementsByTagName('minute')
    if len(intervaltime) > 0:
        configInfo.intervaltime = intervaltime[0].childNodes[0].data

    url_request = root.getElementsByTagName('url_request')
    if len(url_request) > 0:
        configInfo.url_request = url_request[0].childNodes[0].data

    url_reponse = root.getElementsByTagName('url_reponse')
    if len(url_reponse) > 0:
        configInfo.url_reponse = url_reponse[0].childNodes[0].data


    siteInfos = root.getElementsByTagName('siteInfo')
    for siteinfo in siteInfos:
        s_siteInfo = _s_siteInfo()
        _nodes = siteinfo.childNodes
        for _node in _nodes:
            if _node.nodeType == 1:
                _data = _node.childNodes[0].data
                s_siteInfo.siteDict[_node.tagName] = _data
        configInfo.sitelist.append(s_siteInfo)
    return configInfo
