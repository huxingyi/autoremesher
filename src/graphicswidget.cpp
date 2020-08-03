/*
 *  Copyright (c) 2020 Jeremy HU <jeremy-at-dust3d dot org>. All rights reserved. 
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:

 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.

 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 */
#include "graphicswidget.h"
#include "theme.h"

GraphicsWidget::GraphicsWidget()
{
    setStyleSheet("background-color:#252525;");
    setContentsMargins(0, 0, 0, 0);
    setFrameStyle(QFrame::NoFrame);
    
    setAlignment(Qt::AlignCenter);

    setMouseTracking(true);
}

GraphicsWidget::~GraphicsWidget()
{
}

void GraphicsWidget::setModelWidget(PbrShaderWidget *modelWidget)
{
    m_modelWidget = modelWidget;
}

void GraphicsWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_modelWidget && m_modelWidget->inputMouseMoveEventFromOtherWidget(event))
        return;
}

bool GraphicsWidget::inputWheelEventFromOtherWidget(QWheelEvent *event)
{
    if (m_modelWidget && m_modelWidget->inputWheelEventFromOtherWidget(event))
        return true;
    return true;
}

void GraphicsWidget::wheelEvent(QWheelEvent *event)
{
    if (m_modelWidget && m_modelWidget->inputWheelEventFromOtherWidget(event))
        return;
}

void GraphicsWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_modelWidget && m_modelWidget->inputMouseReleaseEventFromOtherWidget(event))
        return;
}

void GraphicsWidget::mousePressEvent(QMouseEvent *event)
{
    if (m_modelWidget && m_modelWidget->inputMousePressEventFromOtherWidget(event))
        return;
}


