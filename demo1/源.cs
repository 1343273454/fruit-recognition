public DataTable Get ? (string? )
{
	DataTable dt = null;
	string sql = null;
	SqlParameter[] cmdParms = {
        new SqlParameter("@?", ? ),
    };//������ֱ���ô��θ���ȫ����Ϊ�鿴sql���ʱ�õ��Ļ���@string������ֱ�ӿ����˾����ֵ
	sql = "select * from tableName";
	dt = SQLHelper.GetDataTable(sql, CommandType.Text, cmdParms);
	if (dt == null)
	{
		return TableTools.CreateTable("error", "û�в������ݿ���в�ѯ������ֵδʵ������Ϊnull��");
	}
	else if (dt.Rows.Count == 0)
	{
		return TableTools.CreateTable("error", "��������ݣ�");
	}
	return dt;
}