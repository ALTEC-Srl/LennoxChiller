Imports LennoxRooftop

Public Class Form1
    Dim init As Boolean
    Dim chiller As New LennoxRooftop.Rooftop()

    Private Sub Button2_Click(sender As Object, e As EventArgs) Handles Button2.Click
        init = chiller.Init()
    End Sub

    Private Sub Button1_Click(sender As Object, e As EventArgs) Handles Button1.Click
        Dim Input As String
        Dim output As String
        If init Then


            Input = "{""MODELID"": ""B6FHD060SP1M"", ""airflowsupply"": 4200, ""airflowexhaust"": 0, ""coiltempdb"": 32.1, ""coiltempwb"": 50,_
""coiltempposthdb"": 22, ""coiltempposthwb"": 18, ""coiltemppostcdb"": 32, ""coiltemppostcwb"":25,  ""iqngn"": 0}"
            output = chiller.GetOptionsPressureDrop(Input)
            Input = "{""MODELID"": ""B6FHC025SP1M"",""SUPPLIERID"": 1, ""fantype"": ""SFHC"", ""airflow"": 3600, ""optionsdp"": 200, ""density"": 1.2, ""temperature"": 20, ""iqngn"": 0}"
            output = chiller.GetFanPerformance(Input)
            MsgBox(output)
        End If
    End Sub
End Class
