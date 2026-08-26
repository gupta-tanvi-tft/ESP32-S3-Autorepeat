import re
with open(r'C:\Users\tanvi-admin\.gemini\antigravity-ide\brain\c9339336-1ce2-4422-8dce-1d292dedff14\.system_generated\steps\700\content.md', 'r', encoding='utf-8') as f:
    text = f.read()
    links = re.findall(r'href=[\'\"]([^\'\"]+\.pdf|[^\'\"]+\.zip)[\'\"]', text)
    for link in links:
        print(link)
