import json
import subprocess
import time
from neo4j import GraphDatabase
from langgraph.prebuilt import create_react_agent
from langchain_neo4j import GraphCypherQAChain,Neo4jGraph
from langchain.agents import Tool
from langchain_core.output_parsers import StrOutputParser
from langchain_core.prompts import PromptTemplate
from langchain_core.runnables import RunnableMap,RunnablePassthrough
from langchain_ollama import ChatOllama

chain = None
cypher_template = None

try:
    subprocess.check_output(['curl','-s',"http://localhost:11434"],shell=True)
    print("ollama is running")
except:
    print("ollama is not running")
    subprocess.Popen(['ollama','serve'],
                    stdout=subprocess.PIPE,
                    stderr=subprocess.PIPE,
                    stdin=subprocess.PIPE,
                    creationflags=subprocess.CREATE_NO_WINDOW)
    time.sleep(5)
    print("ollama is running")

subprocess.Popen(['ollama','run','qwen2.5:7b'],
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                stdin=subprocess.PIPE,
                creationflags=subprocess.CREATE_NO_WINDOW)

lgraph = Neo4jGraph(url="bolt://127.0.0.1:7687",
                    username="neo4j",
                    password="1234qwer",
                    database="neo4j")
print("数据库连接成功")
#1lm补全型模型chat对话型模型
ollm = ChatOllama(model="qwen2.5:7b",temperature=0.1)

print("ollama模型连接成功")

chain = GraphCypherQAChain.from_llm(llm = ollm,
                                    graph = lgraph,
                                    allow_dangerous_requests=True,
                                    verbose = True,
                                    top_k = 10,
                                    validate_cypher=True)
#Trae:解释代码|注释代码|X
def query_disease_auto(disease_name:str)->str:
    global chain
    question=f"请查询疾病'{disease_name}'的所有相关信息,包括症状、药物、检查、科室、治疗方法、推荐和禁忌食物。"
    result = chain.invoke(question)
    return result["result"]

cypher_template =PromptTemplate(input_variables=["query"],
                                    template="""请将以下疾病信息进行综合归纳，生成一段完整、连贯、中文介绍：
                                    {query}""")

def summary_info(info_text:str)->str:
    global cypher_template,chain
    summary_pipeline={
    cypher_template| chain | StrOutputParser()
    }
    return summary_pipeline.invoke({"query":info_text})

disease_tool=Tool(name="疾病信息自动查询",
                    func = query_disease_auto,
                    description="""必须优先使用！用于从医学知识图谱中查询疾病的详细信息，包括症状、药物、检查、科室、治:
                            输入：疾病名称，例如：感冒
                            输出：从知识图谱返回的原始详细信息""")

summary_tool=Tool(name="疾病信息总结工具",
                    func = summary_info,
                    description="""只能在查询完疾病信息后使用！用于把查询到的疾病信息整理成通顺的中文介绍。
                                输入：从知识图谱查询到的原始信息
                                输出：整理后的自然语言介绍""")

agent = create_react_agent(model=ollm,tools=[disease_tool,summary_tool],debug=True)
print("agent创建成功开始工作")
data=agent.invoke({"messages":[("human","帮我整理一下感冒的相关症状")]})
print(data)

subprocess.run(['ollama','stop','qwen2.5:7b'],capture_output=True)

subprocess.run(['taskkill /f /im ollama.exe'],shell=True,capture_output=True)