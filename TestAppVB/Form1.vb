Imports LennoxRooftop
'Imports leelcoilsDLL // da aggiungere nei riferimenti se si vuole provare direttamente da VB, modificare nome assembly in:ExampleExternal
Public Class Form1
    Dim init As Boolean
    Dim rt As New LennoxRooftop.Rooftop()

    Private Sub Button2_Click(sender As Object, e As EventArgs) Handles Button2.Click
        Dim res As Long

        res = rt.Init(0)
        If res > 0 Then
            init = False
            MsgBox("init failed")
            MsgBox(res)
        Else
            init = True
            MsgBox("init ok")
        End If
    End Sub

    Private Sub Button1_Click(sender As Object, e As EventArgs) Handles Button1.Click
        Dim Input As String
        Dim output As String
        If init Then
            '8-05-23 verificato sia k3g, sia k3g in fanwall, sia assiale singolo - verificati diversi codici errori
            Input = "{""modelid"": ""B6010AH025SPF"",""supplierid"": 1, ""fantype"": 1, ""fanoption"": """", ""airflow"": 5000, ""optionsdp"": 100, ""density"": 1.2, ""temperature"": 20, ""iqngn"": 0}"
            If (TextBox1.Text = "") Then
                TextBox1.Text = Input
            Else
                Input = TextBox1.Text
            End If

            output = rt.GetFanPerformance(Input)
            TextBox2.Text = output


            End If
    End Sub

    Private Sub Button3_Click(sender As Object, e As EventArgs) Handles Button3.Click
        Dim Input As String
        Dim output As String
        If init Then
            Input = "{""modelid"": ""B6010AH025SPF"", ""airflowsupply"": 4200, ""airflowexhaust"": 3800, ""coiltempdb"": 32, ""coiltempwb"": 50, ""coiltempposthdb"": 22, ""coiltempposthwb"": 18, ""coiltemppostcdb"": 32, ""coiltemppostcwb"":25,  ""iqngn"": 0}"
            If (TextBox1.Text = "") Then
                TextBox1.Text = Input
            Else
                Input = TextBox1.Text
            End If
            output = rt.GetOptionsPressureDrop(Input)
            TextBox2.Text = output
        End If
    End Sub

    Private Sub Button4_Click(sender As Object, e As EventArgs) Handles Button4.Click
        Dim Input As String
        Dim output As String
        If init Then

            'CString noisesupplyin = doc["noisesupplyin"].GetString();
            'CString noisesupplyout = doc["noisesupplyout"].GetString();
            'CString noiseretin = doc["noiseretin"].GetString();
            'CString noiseretout = doc["noiseretout"].GetString();
            'CString noiseoutin = doc["noiseoutin"].GetString();
            'CString noiseoutout = doc["noiseoutout"].GetString();

            Input = "{""modelid"": ""B6010AH025SPF"",""supplierid"": 1, ""distance"": 3, ""density"": 1.2, ""temperature"": 20, ""options"": [ { ""option"": ""BAUN"", ""value"": 1}, { ""option"": ""FF9B"", ""value"": 1 } ], ""noisesupplyin"": ""68;69;65;63;61;68;61;50"", ""noisesupplyout"": ""69;70;66;66;67;72;63;53"", ""noiseoutin"": ""80;70;60;80;70;60;80;90"", ""noiseoutout"": ""80;70;60;80;70;60;80;90"", ""iqngn"": 0}"
            If (TextBox1.Text = "") Then
                TextBox1.Text = Input
            Else
                Input = TextBox1.Text
            End If
            output = rt.GetNoiseData(Input)
            TextBox2.Text = output
        End If

    End Sub

    Private Sub Button5_Click(sender As Object, e As EventArgs) Handles Button5.Click
        Dim Input As String
        Dim output As String
        If init Then
            Input = "{ ""configuration"":{ ""modelid"" :  ""B6010AH025SPF"", ""supplierid"": 1 , ""coiltype"":2}, ""conditions"": { ""airflow"": 4200, ""airdb"":27, ""airwb"": 19, ""waterin"": 7, ""waterout"":12, ""waterflow"": -1, ""fluidtype"": 1, ""glycoletype"": 0, ""glycoleperc"": 0	}, ""iqngn"":1 } "
            If (TextBox1.Text = "") Then
                TextBox1.Text = Input
            Else
                Input = TextBox1.Text
            End If

            output = rt.GetWaterCoilPerformance(Input)
            TextBox2.Text = output
        End If


        'da aggiungere nei riferimenti se si vuole provare direttamente da VB, modificare nome assembly in:ExampleExternal
        ' Dim calc As leelcoilsDLL.Calculation
        'Dim output As String
        'Dim strJSON As String
        'strJSON = "{""PARAM3"":1,""PARAM4"":true,""PARAM9"":-999999,""PARAM26"":84,""PARAM57"":-999999,""PARAM10"":900,""PARAM70"":0,""PARAM25"":6,""PARAM29"":2.12,""PARAM22"":1,""PARAM41"":1,""PARAM27"":31,""PARAM33"":""G"",""PARAM20"":""4S6"",""OP_01"":1,""OP_09"":2,""OP_03"":1,""OP_02"":1,""PARAM49"":7,""PARAM50"":-999999,""PARAM51"":-999999,""PARAM52"":0,""PARAM36"":42,""PARAM37"":-999999,""PARAM38"":-999999,""PARAM39"":30,""PARAM54"":18612,""PARAM46"":-999999,""PARAM34"":-999999,""PARAM97"":-999999,""PARAM99"":0,""PARAM43"":1,""PARAM89"":-999999,""PARAM90"":-999999,""PARAM91"":-999999,""PARAM92"":-999999,""PARAM93"":-999999,""PARAM40"":6.5,""PARAM45"":12.8,""PARAM55"":-999999,""PARAM47"":-999999,""PARAM95"":-999999,""PARAM87"":0}"
        'calc = New leelcoilsDLL.Calculation()
        'output = calc.StartCalculation(strJSON).Trim()

    End Sub

    Private Sub Button6_Click(sender As Object, e As EventArgs) Handles Button6.Click
        TextBox1.Text = ""
    End Sub
End Class
