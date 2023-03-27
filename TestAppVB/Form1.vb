Imports LennoxRooftop

Public Class Form1
    Private Sub Button1_Click(sender As Object, e As EventArgs) Handles Button1.Click
        Dim Input As String
        Dim output As String


        Input = "{""MODELID"": 1,""SUPPLIERID"": 1, ""airflow"": 1, ""fantypeoption"": 10, ""totaloptionpressuredrop"": 200, ""density"": 1.2, ""temperature"": 20}"
        Dim chiller As New LennoxRooftop.Rooftop()
        output = chiller.GetFanPerformance(Input)
        MsgBox(output)

    End Sub
End Class
