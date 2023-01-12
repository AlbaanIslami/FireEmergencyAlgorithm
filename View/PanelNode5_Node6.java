package View;

import Controller.Controller;
import javax.swing.*;
import javax.swing.border.Border;
import javax.swing.border.CompoundBorder;
import java.awt.*;

/**
 * @author Nezar, Alban on 2022-12-20.
 * @project Project_group_2
 */
public class PanelNode5_Node6 extends JPanel {
    private JList<String> node5;
    private JList<String> node6;
    private Controller controller;

    public PanelNode5_Node6(Controller controller, int width, int height, int margin)
    {
        this.controller = controller;

        Border border = this.getBorder();
        Border emptyBorder = BorderFactory.createEmptyBorder(margin, margin, margin, margin);
        setBorder(new CompoundBorder(border, emptyBorder));
        //data has type Object[]

        Font font = new Font("New Times Roman", Font.PLAIN, 14);
        setBorder(BorderFactory.createTitledBorder("                                  NODE 5                                                                                NODE 6                                   "));

        node5 = new JList<>();
        node6 = new JList<>();

        node5.setFont(font);
        node6.setFont(font);

        JScrollPane s = new JScrollPane(node5);
        JScrollPane s1 = new JScrollPane(node6);

        s.setHorizontalScrollBarPolicy(JScrollPane.HORIZONTAL_SCROLLBAR_ALWAYS);
        s.setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_ALWAYS);
        s.setPreferredSize(new Dimension(350-margin, height-2*margin));
        s1.setHorizontalScrollBarPolicy(JScrollPane.HORIZONTAL_SCROLLBAR_ALWAYS);
        s1.setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_ALWAYS);
        s1.setPreferredSize(new Dimension(350-margin, height-2*margin));

        node5.setSelectionMode(DefaultListSelectionModel.SINGLE_SELECTION);
        node6.setSelectionMode(DefaultListSelectionModel.SINGLE_SELECTION);
        add(s,BorderLayout.WEST);
        add(s1,BorderLayout.CENTER);
    }

    public void updateNode5(String[] node5_info) {
        node5.setListData(node5_info);
    }

    public void updateNode6(String[] node6_info) {
        node6.setListData(node6_info);
    }
}
